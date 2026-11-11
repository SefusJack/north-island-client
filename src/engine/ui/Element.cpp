#include "ui/Element.h"

#include "gfx/Canvas.h"
#include "ui/DrawContext.h"
#include "ui/LayoutError.h"

#include <algorithm>

namespace ui {

    Element::Element() = default;

    const std::string& Element::hintAt(core::Point) const { return m_hint; }

    core::Point Element::hintOrigin(core::Point) const { return {0, 0}; }

    bool Element::raisesOnPressAt(int id) const {
        if (!contains(id)) {
            return false;
        }
        if (raisesOnPress()) {
            return true;
        }
        return std::any_of(m_children.begin(), m_children.end(), [id](const auto& child) { return child->raisesOnPressAt(id); });
    }

    bool Element::raisesAbovePopupsAt(int id) const {
        if (!contains(id)) {
            return false;
        }
        if (raisesAbovePopups()) {
            return true;
        }
        return std::any_of(m_children.begin(), m_children.end(), [id](const auto& child) { return child->raisesAbovePopupsAt(id); });
    }

    bool Element::contains(int id) const {
        if (m_id == id) {
            return true;
        }
        return std::any_of(m_children.begin(), m_children.end(), [id](const auto& child) { return child->contains(id); });
    }

    void Element::raiseChildHolding(int id) {
        const auto holder = std::find_if(m_children.begin(), m_children.end(), [id](const auto& child) { return child->contains(id); });
        if (holder == m_children.end()) {
            return;
        }
        if ((*holder)->isTopmost()) {
            std::rotate(holder, holder + 1, m_children.end());
            return;
        }
        const auto firstTopmost = std::find_if(m_children.begin(), m_children.end(), [](const auto& child) { return child->isTopmost(); });
        if (firstTopmost < holder) {
            std::rotate(firstTopmost, holder, holder + 1);
        } else {
            std::rotate(holder, holder + 1, firstTopmost);
        }
    }

    void Element::raiseChildHoldingAbovePopups(int id) {
        const auto holder = std::find_if(m_children.begin(), m_children.end(), [id](const auto& child) { return child->contains(id); });
        if (holder != m_children.end()) {
            std::rotate(holder, holder + 1, m_children.end());
        }
    }

    void Element::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        (void)context;
        if (name == "Name") {
            m_name = value;
        } else if (name == "Bounds") {
            m_bounds = LoadContext::parseRect(value);
            m_sizeGiven = true;
        } else if (name == "Position") {
            setPosition(LoadContext::parsePoint(value));
        } else if (name == "Size") {
            const core::Point size = LoadContext::parsePoint(value);
            m_bounds.width = size.x;
            m_bounds.height = size.y;
            m_sizeGiven = true;
        } else if (name == "Visible") {
            m_visible = LoadContext::parseBool(value);
        } else if (name == "Enabled") {
            m_enabled = LoadContext::parseBool(value);
        } else if (name == "Cursor") {
            if (value == "Normal") {
                m_cursor = CursorKind::Normal;
            } else if (value == "Hand") {
                m_cursor = CursorKind::Hand;
            } else {
                throw LayoutError("unknown cursor \"" + value + "\"");
            }
        } else if (name == "ClickSound") {
            m_clickSound = value;
        } else if (name == "ReleaseSound") {
            m_releaseSound = value;
        } else if (name == "Topmost") {
            m_topmost = LoadContext::parseBool(value);
        } else if (name == "HudScale") {
            m_hudScaled = LoadContext::parseBool(value);
        } else if (name == "Anchor") {
            static const std::map<std::string, Anchor> kAnchors{
                {"None", Anchor::None},
                {"TopLeft", Anchor::TopLeft},
                {"Top", Anchor::Top},
                {"TopRight", Anchor::TopRight},
                {"Left", Anchor::Left},
                {"Center", Anchor::Center},
                {"Right", Anchor::Right},
                {"BottomLeft", Anchor::BottomLeft},
                {"Bottom", Anchor::Bottom},
                {"BottomRight", Anchor::BottomRight},
                {"Fill", Anchor::Fill}
            };
            const auto found = kAnchors.find(value);
            if (found == kAnchors.end()) {
                throw LayoutError("unknown anchor \"" + value + "\"");
            }
            m_anchor = found->second;
        } else if (name == "RaisesOnPress") {
            m_raisesOnPress = LoadContext::parseBool(value);
        } else if (name == "Hint") {
            m_hint = value;
        } else if (m_events.count(name) != 0) {
            m_handlers[name] = value;
        } else {
            throw LayoutError("unknown attribute " + name);
        }
    }

    void Element::finishLoading(const LoadContext& context) { (void)context; }

    void Element::update(UiContext& context) {
        for (const auto& child : m_children) {
            if (child->isVisible()) {
                child->update(context);
            }
        }
    }

    void Element::drawTree(DrawContext& context, core::Point origin) {
        if (!m_visible) {
            return;
        }
        const core::Point position = origin + m_bounds.position();
        draw(context, position);
        if (clipsChildren()) {
            context.canvas.setClip(core::Rect{position.x, position.y, m_bounds.width, m_bounds.height});
        }
        for (const auto& child : m_children) {
            child->drawTree(context, position);
        }
        if (clipsChildren()) {
            context.canvas.setClip(std::nullopt);
        }
    }

    void Element::drawOverlayTree(DrawContext& context, core::Point origin) {
        if (!m_visible) {
            return;
        }
        const core::Point position = origin + m_bounds.position();
        drawOverlay(context, position);
        for (const auto& child : m_children) {
            child->drawOverlayTree(context, position);
        }
    }

    void Element::drawOverlay(DrawContext& context, core::Point screenPosition) {
        (void)context;
        (void)screenPosition;
    }

    bool Element::hitTestOverlay(core::Point local) const {
        (void)local;
        return false;
    }

    void Element::draw(DrawContext& context, core::Point screenPosition) {
        (void)context;
        (void)screenPosition;
    }

    bool Element::hitTest(core::Point local, const gfx::PictureLibrary& pictures) const {
        (void)local;
        (void)pictures;
        return false;
    }

    void Element::raise(UiContext& context, const std::string& event, int index, core::Point position) const {
        const auto handler = m_handlers.find(event);
        if (handler != m_handlers.end()) {
            EventArgs args{m_name, index, position};
            args.ctrl = context.ctrlHeld;
            context.raise(handler->second, args);
        }
    }

    void Element::raise(UiContext& context, const std::string& event, EventArgs args) const {
        const auto handler = m_handlers.find(event);
        if (handler != m_handlers.end()) {
            args.source = m_name;
            args.ctrl = context.ctrlHeld;
            context.raise(handler->second, args);
        }
    }

    void Element::onMouseDown(UiContext&, MouseButton, core::Point) {}

    void Element::onMouseUp(UiContext&, MouseButton, core::Point) {}

    void Element::onMouseDrag(UiContext&, core::Point) {}

    void Element::onCaptureReleased(UiContext&) {}

    void Element::onDropped(
        UiContext&,
        int,
        std::optional<std::reference_wrapper<const Element>>,
        int,
        core::Point,
        core::Point
    ) {}

    int Element::dropIndexAt(core::Point) const { return -1; }

    void Element::onDoubleClick(UiContext&, core::Point) {}

    void Element::onMouseWheel(UiContext&, int) {}

    void Element::onKeyDown(UiContext&, const KeyEvent&) {}

    void Element::onKeyUp(UiContext&, const KeyEvent&) {}

    void Element::onTextInput(UiContext&, const std::string&) {}

    namespace {

        /// @brief The halves of the window's extra width and height an anchor keeps to.
        /// @param anchor The anchor.
        /// @return x and y shares: 0 the left or top edge, 1 the middle, 2 the right or bottom edge.
        core::Point anchorShares(Element::Anchor anchor) {
            switch (anchor) {
                case Element::Anchor::TopLeft:
                case Element::Anchor::Fill: return {0, 0}; // top left corner (Fill also grows from there)
                case Element::Anchor::Top: return {1, 0}; // top edge, middle horizontally
                case Element::Anchor::TopRight: return {2, 0}; // top right corner
                case Element::Anchor::Left: return {0, 1}; // left edge, middle vertically
                case Element::Anchor::Right: return {2, 1}; // right edge, middle vertically
                case Element::Anchor::BottomLeft: return {0, 2}; // bottom left corner
                case Element::Anchor::Bottom: return {1, 2}; // bottom edge, middle horizontally
                case Element::Anchor::BottomRight: return {2, 2}; // bottom right corner
                case Element::Anchor::None:
                case Element::Anchor::Center: break; // the middle (the default share)
            }
            return {1, 1};
        }

    }

    core::Point Element::anchorPoint() const {
        const core::Point shares = anchorShares(m_anchor);
        // shares are in halves: 0 start, 1 middle, 2 end
        return {m_bounds.x + m_bounds.width * shares.x / 2, m_bounds.y + m_bounds.height * shares.y / 2};
    }

    void Element::applyView(const core::Rect& view, core::Point parentShift) {
        constexpr int kScreenWidth = 800; // px width of the original game screen the layouts were made for
        constexpr int kScreenHeight = 600; // px height of the original game screen
        const core::Point shares = anchorShares(m_anchor);
        const int across = shares.x;
        const int down = shares.y;
        core::Point shift;
        core::Point growth;
        if (m_anchor != Anchor::None) {
            const core::Point absolute{view.x + (view.width - kScreenWidth) * across / 2, view.y + (view.height - kScreenHeight) * down / 2};
            shift = absolute - parentShift;
        }
        if (m_anchor == Anchor::Fill) {
            growth = {view.width - kScreenWidth, view.height - kScreenHeight};
        }
        m_bounds.x += shift.x - m_viewShift.x;
        m_bounds.y += shift.y - m_viewShift.y;
        m_bounds.width += growth.x - m_viewGrowth.x;
        m_bounds.height += growth.y - m_viewGrowth.y;
        m_viewShift = shift;
        m_viewGrowth = growth;
        for (const auto& child : m_children) {
            child->applyView(view, parentShift + shift);
        }
    }

}
