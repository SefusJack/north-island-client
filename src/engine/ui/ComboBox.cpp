#include "ui/ComboBox.h"

#include <algorithm>

namespace ui {

    ComboBox::ComboBox() {
        declareEvent("Select");
        declareEvent("ItemRightClick");
    }

    void ComboBox::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "DropButton") {
            m_buttonName = value;
        } else if (name == "DropButtonOffset") {
            m_buttonOffset = LoadContext::parsePoint(value);
        } else if (name == "ListFontColor") {
            m_listFontColor = LoadContext::parseColor(value);
        } else if (name == "ListHighlightColor") {
            m_listHighlight = LoadContext::parseColor(value);
        } else if (name == "MaxItems") {
            m_maxItems = static_cast<std::size_t>(std::max(1, LoadContext::parseInt(value)));
        } else {
            TextBox::applyAttribute(name, value, context);
        }
    }

    void ComboBox::finishLoading(const LoadContext& context) {
        m_button = context.picture(m_buttonName);
        if (m_button.isValid()) {
            const gfx::Picture& picture = context.pictureData(m_button);
            m_buttonWidth = picture.width();
            m_buttonHeight = picture.height() / 3; // 3 = button states stacked in the picture: normal, hover, pressed
        }
        TextBox::finishLoading(context);
    }

    core::Rect ComboBox::buttonArea() const { return {bounds().width + m_buttonOffset.x, m_buttonOffset.y, m_buttonWidth, m_buttonHeight}; }

    core::Rect ComboBox::listArea() const {
        const int rows = std::min(static_cast<int>(m_items.size()), kVisibleRows);
        // 62 = list height in pixels when it scrolls; 2 = border padding; 1 = gap below the field
        const int height = static_cast<int>(m_items.size()) > kVisibleRows ? 62 : rows * kRowHeight + 2;
        return {0, bounds().height + 1, bounds().width, height};
    }

    int ComboBox::rowAt(core::Point local) const {
        const int top = bounds().height + 4; // 4 = pixels from the field bottom to the first list row
        if (!m_open || local.x < 0 || local.x >= bounds().width || local.y < top) {
            return -1;
        }
        const int row = (local.y - top) / kRowHeight;
        if (row >= kVisibleRows || m_top + row >= static_cast<int>(m_items.size())) {
            return -1;
        }
        return m_top + row;
    }

    bool ComboBox::hitTest(core::Point local, const gfx::PictureLibrary& pictures) const {
        return buttonArea().contains(local) || TextBox::hitTest(local, pictures);
    }

    bool ComboBox::hitTestOverlay(core::Point local) const { return m_open && listArea().contains(local); }

    void ComboBox::setItems(std::vector<std::string> items) {
        if (items.size() > m_maxItems) {
            items.resize(m_maxItems);
        }
        m_items = std::move(items);
        m_top = 0;
        if (m_items.empty()) {
            m_open = false;
        }
    }

    void ComboBox::removeItem(int index) {
        if (index >= 0 && index < static_cast<int>(m_items.size())) {
            m_items.erase(m_items.begin() + index);
        }
        if (m_items.empty()) {
            m_open = false;
        }
    }

    void ComboBox::addToHistory(const std::string& text) {
        const auto found = std::find(m_items.begin(), m_items.end(), text);
        if (found != m_items.end()) {
            m_items.erase(found);
        } else if (m_items.size() >= m_maxItems) {
            m_items.pop_back();
        }
        m_items.insert(m_items.begin(), text);
    }

    void ComboBox::toggle() {
        if (m_items.empty()) {
            return;
        }
        m_open = !m_open;
        m_top = 0;
    }

    void ComboBox::onMouseDown(UiContext& context, MouseButton button, core::Point local) {
        if (button == MouseButton::Left && buttonArea().contains(local)) {
            m_buttonPressed = true;
            return;
        }
        TextBox::onMouseDown(context, button, local);
    }

    void ComboBox::onMouseUp(UiContext& context, MouseButton button, core::Point local) {
        if (button == MouseButton::Left && m_buttonPressed && buttonArea().contains(local)) {
            m_buttonPressed = false;
            toggle();
            return;
        }
        m_buttonPressed = false;
        const int index = rowAt(local);
        if (index < 0) {
            return;
        }
        if (button == MouseButton::Left) {
            setText(m_items[static_cast<std::size_t>(index)]);
            m_open = false;
            raise(context, "Select", index);
        } else {
            raise(context, "ItemRightClick", index);
        }
    }

    void ComboBox::onMouseWheel(UiContext& context, int steps) {
        (void)context;
        if (!m_open) {
            return;
        }
        const int maxTop = std::max(0, static_cast<int>(m_items.size()) - kVisibleRows);
        m_top = std::clamp(m_top - steps, 0, maxTop);
    }

    void ComboBox::draw(DrawContext& context, core::Point screenPosition) {
        TextBox::draw(context, screenPosition);
        const bool hovered = context.hoveredId == id();
        const core::Point local = context.mouse - screenPosition;

        const core::Rect button = buttonArea();
        int buttonState = 0;
        if (hovered && button.contains(local)) {
            buttonState = m_buttonPressed ? 2 : 1; // picture state: 2 = pressed, 1 = hover
        } else {
            m_buttonPressed = false;
        }
        context.canvas.drawPicture(m_button, screenPosition + button.position(), core::Rect{0, buttonState * m_buttonHeight, m_buttonWidth, m_buttonHeight});

        if (context.focusedId != id()) {
            m_open = false;
        }
    }

    void ComboBox::drawOverlay(DrawContext& context, core::Point screenPosition) {
        if (!m_open) {
            return;
        }
        const bool hovered = context.hoveredId == id();
        const core::Point local = context.mouse - screenPosition;
        const core::Rect box = listArea().translated(screenPosition);
        // list background is opaque white; its outline is navy #000080
        context.canvas.fillRect(box, {255, 255, 255, 255});
        context.canvas.drawRectOutline(box, core::Color::fromRgb(0x000080));
        const int hoveredIndex = hovered ? rowAt(local) : -1;
        for (int row = 0; row < kVisibleRows && m_top + row < static_cast<int>(m_items.size()); ++row) {
            const int y = screenPosition.y + bounds().height + 4 + row * kRowHeight; // 4 = pixels from the field bottom to the first list row
            if (m_top + row == hoveredIndex) {
                context.canvas.fillRectAlpha(
                    {screenPosition.x + 1, y - 2, bounds().width - 2, kRowHeight},
                    m_listHighlight
                ); // highlight inset 1 px each side and raised 2 px over the text
            }
            // 3 = text left padding in pixels; 512 = text max width; 10 = line height
            context.canvas.drawText(
                {screenPosition.x + 3, y},
                m_items[static_cast<std::size_t>(m_top + row)],
                {m_listFontColor, gfx::TextEffect::Plain, std::nullopt},
                512,
                10
            );
        }
    }

}
