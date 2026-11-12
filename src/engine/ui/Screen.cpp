#include "ui/Screen.h"

#include "core/FileSystem.h"
#include "gfx/TextStyle.h"
#include "ui/LayoutLoader.h"
#include "ui/ListBox.h"
#include "ui/Memo.h"
#include "ui/ScrollBar.h"
#include "ui/TextBox.h"
#include "ui/Tooltip.h"
#include "xml/Parser.h"

#include <cmath>
#include <unordered_set>

namespace ui {

    namespace {

        /// @brief Gives every element of a tree a unique id, depth first.
        /// @param element Root of the tree.
        /// @param next Next free id; advanced past the ids handed out.
        void assignIds(Element& element, int& next) {
            element.setId(next++);
            for (const auto& child : element.children()) {
                assignIds(*child, next);
            }
        }

        /// @brief Finds the first element of that name in a tree, depth first.
        /// @param element Root of the tree.
        /// @param name Element name.
        /// @return The element, or nothing.
        std::optional<std::reference_wrapper<Element>> findNamed(Element& element, const std::string& name) {
            if (element.name() == name) {
                return std::ref(element);
            }
            for (const auto& child : element.children()) {
                if (auto found = findNamed(*child, name)) {
                    return found;
                }
            }
            return std::nullopt;
        }

        /// @brief Collects the rectangles of the visible windows (ui::Window) in a tree; does not descend into windows.
        /// @param element Root of the tree.
        /// @param origin Screen position of the element's parent.
        /// @param windows Receives the window rectangles in the screen's coordinates.
        void collectWindows(const Element& element, core::Point origin, std::vector<core::Rect>& windows) {
            if (!element.isVisible()) {
                return;
            }
            const core::Point position = origin + element.bounds().position();
            if (dynamic_cast<const Window*>(&element) != nullptr) {
                windows.push_back({position.x, position.y, element.bounds().width, element.bounds().height});
                return;
            }
            for (const auto& child : element.children()) {
                collectWindows(*child, position, windows);
            }
        }

        /// @brief Finds the topmost element whose hit test (or overlay hit test) takes the point: children before their parent, later children before earlier ones.
        /// @param element Root of the tree.
        /// @param origin Screen position of the element's parent.
        /// @param mouse Mouse position in the layout's coordinates.
        /// @param pictures Picture library for pixel hit tests.
        /// @param overlay Whether to test the overlays (open drop-down lists) instead of the elements.
        /// @return The element id, 0 for none.
        int topmostHit(
            Element& element,
            core::Point origin,
            core::Point mouse,
            const gfx::PictureLibrary& pictures,
            bool overlay
        ) {
            if (!element.isVisible()) {
                return 0;
            }
            const core::Point position = origin + element.bounds().position();
            const auto& children = element.children();
            const bool childrenShown =
                !element.clipsChildren() || core::Rect{position.x, position.y, element.bounds().width, element.bounds().height}.contains(mouse);
            for (auto child = children.rbegin(); childrenShown && child != children.rend(); ++child) {
                if (const int id = topmostHit(**child, position, mouse, pictures, overlay)) {
                    return id;
                }
            }
            const core::Point local = mouse - position;
            const bool hit = overlay ? element.hitTestOverlay(local) : element.hitTest(local, pictures);
            return hit ? element.id() : 0;
        }

    }

    Screen::Screen(
        gfx::PictureLibrary& pictures
    )
        : m_root(std::make_unique<Element>()),
          m_pictures(pictures) {
        m_context.focusByName = [this](const std::string& name) { focus(name); };
        m_context.bringToFront = [this](int id) { m_root->raiseChildHolding(id); };
        m_context.bringAbovePopups = [this](int id) { m_root->raiseChildHoldingAbovePopups(id); };
    }

    Element& Screen::load(const std::filesystem::path& layoutFile, const LoadContext& loadContext) {
        const auto data = core::readFile(layoutFile);
        if (!data) {
            throw LayoutError("cannot read " + layoutFile.string());
        }
        return load(LayoutSource{layoutFile.filename().string(), core::toString(*data)}, loadContext);
    }

    void Screen::prefetchPictures(const xml::Node& node, std::vector<std::string>& started) {
        // the attributes that name pictures (Window, ImageButton, ImageView, CheckBox, ComboBox, IconGrid, Memo, Panel, ScrollBar, ...)
        static const std::unordered_set<std::string> kPictureAttributes{
            "Picture", "Animation", "Caret", "CellPicture", "CheckedPicture", "DropButton", "ScrollBar", "Skin", "Thumb", "Track"
        };
        for (const auto& [name, value] : node.attributes()) {
            if (kPictureAttributes.count(name) != 0 && m_pictures.prefetch(value)) {
                started.push_back(value);
            }
        }
        for (const xml::Node& child : node.children()) {
            prefetchPictures(child, started);
        }
    }

    Element& Screen::load(const LayoutSource& source, const LoadContext& loadContext) {
        std::unique_ptr<Element> layout;
        std::vector<std::string> prefetched; // pictures decoding on workers while the layout is built
        const auto forgetPrefetched = [&] {
            for (const std::string& name : prefetched) {
                m_pictures.cancelPrefetch(name); // no-op for the ones the layout used
            }
        };
        try {
            const xml::Node root = xml::Parser::parse(source.text);
            prefetchPictures(root, prefetched);
            layout = LayoutLoader::build(root, loadContext);
        } catch (const std::exception& error) {
            forgetPrefetched();
            throw LayoutError(source.name + ": " + error.what());
        }
        forgetPrefetched();
        assignIds(*layout, m_nextId);
        layout->applyView(m_view, {});
        Element& added = *layout;
        m_root->addChild(std::move(layout));
        return added;
    }

    void Screen::setView(const core::Rect& view) {
        m_view = view;
        m_context.view = view;
        for (const auto& layout : m_root->children()) {
            layout->applyView(view, {});
        }
    }

    Element& Screen::findByName(const std::string& name) {
        if (m_namedAt != Element::treeChanges()) {
            m_named.clear(); // a tree grew, was reordered or renamed: the first match may differ now
            m_namedAt = Element::treeChanges();
        }
        if (const auto known = m_named.find(name); known != m_named.end()) {
            return *known->second;
        }
        auto found = findNamed(*m_root, name);
        if (!found) {
            throw LayoutError("the layout has no element " + name);
        }
        m_named.emplace(name, &found->get());
        return found->get();
    }

    std::optional<Screen::Located> Screen::locate(int id) {
        if (id == 0) {
            return std::nullopt;
        }
        std::function<std::optional<Located>(Element&, core::Point, LayoutRef)> search =
            [&](Element& element, core::Point origin, LayoutRef layout) -> std::optional<Located> {
            if (!element.isVisible()) {
                return std::nullopt;
            }
            const core::Point position = origin + element.bounds().position();
            if (element.id() == id) {
                return Located{element, position, layout};
            }
            for (const auto& child : element.children()) {
                if (auto found = search(*child, position, layout ? layout : LayoutRef(std::cref(*child)))) {
                    return found;
                }
            }
            return std::nullopt;
        };
        return search(*m_root, {0, 0}, std::nullopt);
    }

    core::Point Screen::mouseIn(const Element& element) {
        const auto located = locate(element.id());
        return located ? toLayout(m_mouse, located->layout) : m_mouse;
    }

    float Screen::scaleOf(LayoutRef layout) const { return layout && layout->get().isHudScaled() ? m_hudScale : 1.0f; }

    core::Point Screen::toLayout(core::Point point, LayoutRef layout) const {
        const float scale = scaleOf(layout);
        if (scale == 1.0f) {
            return point;
        }
        const core::Point pivot = layout->get().anchorPoint();
        return {pivot.x + static_cast<int>(std::floor((point.x - pivot.x) / scale)), pivot.y + static_cast<int>(std::floor((point.y - pivot.y) / scale))};
    }

    core::Point Screen::fromLayout(core::Point point, LayoutRef layout) const {
        const float scale = scaleOf(layout);
        if (scale == 1.0f) {
            return point;
        }
        const core::Point pivot = layout->get().anchorPoint();
        return {pivot.x + static_cast<int>(std::lround((point.x - pivot.x) * scale)), pivot.y + static_cast<int>(std::lround((point.y - pivot.y) * scale))};
    }

    int Screen::hitAt(core::Point position, const Element** layoutOut) {
        if (layoutOut != nullptr) {
            *layoutOut = nullptr;
        }
        const auto& layouts = m_root->children();
        for (const bool overlay : {true, false}) {
            for (auto layout = layouts.rbegin(); layout != layouts.rend(); ++layout) {
                if (!kept(**layout)) {
                    continue;
                }
                if (const int id = topmostHit(**layout, {0, 0}, toLayout(position, std::cref(**layout)), m_pictures, overlay)) {
                    if (layoutOut != nullptr) {
                        *layoutOut = layout->get();
                    }
                    return id;
                }
            }
        }
        return 0;
    }

    void Screen::refreshHover() { m_hoveredId = hitAt(m_mouse, &m_hoveredLayout); }

    void Screen::focus(const std::string& name) { m_context.focusedId = findByName(name).id(); }

    void Screen::mouseMove(core::Point position, bool leftHeld) {
        m_mouse = position;
        refreshHover();
        if (leftHeld && m_capturedId != 0) {
            if (auto captured = locate(m_capturedId)) {
                captured->element.get().onMouseDrag(m_context, localMouse(*captured, position));
            }
        }
    }

    void Screen::mouseDown(core::Point position, MouseButton button, int clicks, bool ctrl) {
        m_mouse = position;
        m_context.ctrlHeld = ctrl;
        refreshHover();
        m_context.focusedId = m_hoveredId;
        auto target = locate(m_hoveredId);
        if (!target) {
            return;
        }
        Element& element = target->element.get();
        if (button == MouseButton::Left) {
            m_capturedId = m_hoveredId;
        }
        if (element.isEnabled() && !element.clickSound().empty() && m_context.playSound) {
            m_context.playSound(element.clickSound());
        }
        if (m_root->raisesOnPressAt(m_hoveredId)) {
            if (m_root->raisesAbovePopupsAt(m_hoveredId)) {
                m_root->raiseChildHoldingAbovePopups(m_hoveredId);
            } else {
                m_root->raiseChildHolding(m_hoveredId);
            }
        }
        const core::Point local = localMouse(*target, position);
        // 2: a double click (second press of a pair)
        if (clicks >= 2) {
            clicks = clicks % 2 == 0 ? 2 : 1;
        }
        if (button == MouseButton::Left) {
            m_pressWasDouble = clicks >= 2;
        }
        if (button == MouseButton::Left && clicks >= 2) {
            element.onDoubleClick(m_context, local);
        }
        element.onMouseDown(m_context, button, local);
    }

    void Screen::mouseUp(core::Point position, MouseButton button, bool ctrl) {
        m_mouse = position;
        m_context.ctrlHeld = ctrl;
        refreshHover();
        if (button == MouseButton::Left && m_context.drag) {
            dropDragged(position);
        }
        const int captured = button == MouseButton::Left ? m_capturedId : 0;
        if (button == MouseButton::Left) {
            m_capturedId = 0;
        }
        if (m_releaseConsumed) {
            m_releaseConsumed = false;
            return;
        }
        if (auto target = locate(m_hoveredId)) {
            Element& element = target->element.get();
            if (button == MouseButton::Left && element.isEnabled() && !element.releaseSound().empty() && m_context.playSound) {
                m_context.playSound(element.releaseSound());
            }
            m_context.releaseOfDoubleClick = button == MouseButton::Left && m_pressWasDouble;
            element.onMouseUp(m_context, button, localMouse(*target, position));
            m_context.releaseOfDoubleClick = false;
        }
        if (captured != 0 && captured != m_hoveredId) {
            if (auto previous = locate(captured)) {
                previous->element.get().onCaptureReleased(m_context);
            }
        }
    }

    void Screen::dropDragged(core::Point position) {
        const DragItem drag = *m_context.drag;
        m_context.drag.reset();
        const auto source = locate(drag.sourceId);
        if (!source) {
            return;
        }
        const auto target = locate(m_hoveredId);
        std::optional<std::reference_wrapper<const Element>> targetElement;
        if (target) {
            targetElement = std::cref(target->element.get());
        }
        const int targetIndex = target ? targetElement->get().dropIndexAt(localMouse(*target, position)) : -1;
        source->element.get().onDropped(m_context, drag.sourceIndex, targetElement, targetIndex, position, drag.grab);
    }

    void Screen::mouseWheel(int steps) {
        if (auto target = locate(m_context.focusedId)) {
            target->element.get().onMouseWheel(m_context, steps);
        }
    }

    void Screen::keyDown(const KeyEvent& key) {
        m_heldKeys.insert(key.key);
        if (m_keyPreview && m_keyPreview(key)) {
            return;
        }
        if (auto target = locate(m_context.focusedId)) {
            target->element.get().onKeyDown(m_context, key);
        }
    }

    std::optional<std::reference_wrapper<Window>> Screen::topEscapeWindow(Element& parent) {
        for (auto child = parent.children().rbegin(); child != parent.children().rend(); ++child) {
            Element& element = **child;
            if (!element.isVisible()) {
                continue;
            }
            if (auto* window = dynamic_cast<Window*>(&element); window != nullptr && window->closesOnEscape()) {
                return *window;
            }
            if (const auto inner = topEscapeWindow(element)) {
                return inner;
            }
        }
        return std::nullopt;
    }

    bool Screen::closeTopOnEscape() {
        if (const auto window = topEscapeWindow(*m_root)) {
            window->get().raiseEscape(m_context);
            return true;
        }
        return false;
    }

    void Screen::keyUp(const KeyEvent& key) {
        m_heldKeys.erase(key.key);
        if (auto target = locate(m_context.focusedId)) {
            target->element.get().onKeyUp(m_context, key);
        }
    }

    void Screen::textInput(const std::string& text) {
        if (m_textFallback) {
            const auto holder = locate(m_context.focusedId);
            const bool textField =
                holder && (dynamic_cast<const TextBox*>(&holder->element.get()) != nullptr || dynamic_cast<const Memo*>(&holder->element.get()) != nullptr);
            if (!textField) {
                m_textFallback();
            }
        }
        if (auto target = locate(m_context.focusedId)) {
            target->element.get().onTextInput(m_context, text);
        }
    }

    void Screen::collectListBoxes(Element& element) {
        if (auto* list = dynamic_cast<ListBox*>(&element)) {
            m_listBoxes.push_back(list);
        }
        for (const auto& child : element.children()) {
            collectListBoxes(*child);
        }
    }

    void Screen::syncScrollBars() {
        if (m_listBoxesAt != Element::treeChanges()) {
            m_listBoxes.clear(); // in tree order, as the scroll bars were synced before
            collectListBoxes(*m_root);
            m_listBoxesAt = Element::treeChanges();
        }
        for (ListBox* list : m_listBoxes) {
            if (!list->scrollBarName().empty()) {
                list->syncScrollBar(get<ScrollBar>(list->scrollBarName()));
            }
        }
    }

    void Screen::update(std::uint64_t nowMs) {
        m_context.nowMs = nowMs;
        m_root->update(m_context);
        syncScrollBars();
        refreshHover();
    }

    std::vector<core::Rect> Screen::shownWindows() const {
        std::vector<core::Rect> windows;
        for (const auto& layout : m_root->children()) {
            if (kept(*layout)) {
                collectWindows(*layout, {0, 0}, windows);
            }
        }
        return windows;
    }

    void Screen::draw(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (m_hoveredLayout != nullptr && !kept(*m_hoveredLayout)) {
            // update found the hovered element without this filter (party member screens get it only to draw), in a
            // layout the filter hides: find it again among the kept layouts, so hidden layouts give no highlight or
            // hint. A hit in a kept layout is already what the filtered search would find: every layout searched
            // before it gave no hit.
            refreshHover();
        }
        DrawContext context{canvas, m_hoveredId, m_context.focusedId, m_mouse, nowMs};
        const auto drawLayouts = [&](bool overlay) {
            for (const auto& layout : m_root->children()) {
                if (!kept(*layout)) {
                    continue;
                }
                const float scale = scaleOf(std::cref(*layout));
                if (scale != 1.0f) {
                    canvas.beginScale(layout->anchorPoint(), scale);
                    context.mouse = toLayout(m_mouse, std::cref(*layout));
                }
                if (overlay) {
                    layout->drawOverlayTree(context, {0, 0});
                } else {
                    layout->drawTree(context, {0, 0});
                }
                if (scale != 1.0f) {
                    canvas.endScale();
                    context.mouse = m_mouse;
                }
            }
        };
        drawLayouts(false);
        drawLayouts(true);
        if (m_context.drag) {
            canvas.drawPicture(m_context.drag->picture, m_mouse - m_context.drag->grab, m_context.drag->source);
            return;
        }
        drawHint(canvas);
    }

    void Screen::drawHint(gfx::Canvas& canvas) {
        const auto target = locate(m_hoveredId);
        if (!target || (target->layout && !kept(target->layout->get()))) {
            return; // nothing hovered, or in a layout the filter hides
        }
        const core::Point local = localMouse(*target, m_mouse);
        drawHintBox(canvas, fromLayout(target->screenPosition + target->element.get().hintOrigin(local), target->layout), target->element.get().hintAt(local));
    }

    CursorKind Screen::cursor() {
        if (auto hovered = locate(m_hoveredId)) {
            return hovered->element.get().cursor();
        }
        return CursorKind::Normal;
    }

    bool Screen::isTyping() {
        const auto target = locate(m_context.focusedId);
        return target && target->element.get().takesText();
    }

    bool Screen::isShown(const std::string& name) {
        std::function<bool(const Element&)> search = [&](const Element& element) {
            if (!element.isVisible()) {
                return false;
            }
            if (element.name() == name) {
                return true;
            }
            for (const auto& child : element.children()) {
                if (search(*child)) {
                    return true;
                }
            }
            return false;
        };
        return search(*m_root);
    }

    bool Screen::isTopmostAt(const std::string& name, core::Point position) {
        const auto found = findNamed(*m_root, name);
        return found && hitAt(position) == found->get().id();
    }

}
