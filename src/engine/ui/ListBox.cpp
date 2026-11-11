#include "ui/ListBox.h"

#include "gfx/TextRenderer.h"
#include "ui/ScrollBar.h"
#include "ui/TextWrap.h"

#include <algorithm>

namespace ui {

    ListBox::ListBox() {
        declareEvent("Select");
        declareEvent("DoubleClick");
        declareEvent("RightClick");
        setCursor(CursorKind::Hand);
    }

    void ListBox::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "RowHeight") {
            m_rowHeight = std::max(1, LoadContext::parseInt(value));
        } else if (name == "FontColor") {
            m_fontColor = LoadContext::parseColor(value);
        } else if (name == "HighlightColor") {
            m_highlightColor = LoadContext::parseColor(value);
        } else if (name == "FrameColor") {
            m_frameColor = LoadContext::parseColor(value);
        } else if (name == "ShowSelection") {
            m_showSelection = LoadContext::parseBool(value);
        } else if (name == "ShowHoverBar") {
            m_showHoverBar = LoadContext::parseBool(value);
        } else if (name == "FocusFrame") {
            m_focusFrame = LoadContext::parseBool(value);
        } else if (name == "AlwaysFrame") {
            m_alwaysFrame = LoadContext::parseBool(value);
        } else if (name == "Wheel") {
            m_wheel = LoadContext::parseBool(value);
        } else if (name == "ToggleSelect") {
            m_toggleSelect = LoadContext::parseBool(value);
        } else if (name == "TextOffset") {
            m_textOffset = LoadContext::parsePoint(value);

        } else if (name == "TextShadow") {
            m_textShadow = LoadContext::parseBool(value);
        } else if (name == "WrapColumns") {
            m_wrapColumns = LoadContext::parseInt(value);
        } else if (name == "HoverBarTrim") {
            m_hoverBarTrim = LoadContext::parsePoint(value);
        } else if (name == "ScrollBar") {
            m_scrollBarName = value;
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    bool ListBox::hitTest(core::Point local, const gfx::PictureLibrary& pictures) const {
        (void)pictures;
        return core::Rect{0, 0, bounds().width, bounds().height}.contains(local);
    }

    int ListBox::visibleRows() const {
        int rows = bounds().height / m_rowHeight;
        if (bounds().height - m_rowHeight * rows >= m_rowHeight) {
            ++rows;
        }
        return rows;
    }

    int ListBox::rowAt(core::Point local) const {
        if (local.y < 0 || local.x < 0 || local.x >= bounds().width) {
            return -1;
        }
        const int row = local.y / m_rowHeight;
        if (row >= visibleRows() || m_top + row >= itemCount()) {
            return -1;
        }
        return row;
    }

    void ListBox::clear() {
        m_items.clear();
        m_selected = -1;
        m_top = 0;
    }

    void ListBox::addItem(const std::string& text, std::optional<core::Color> color) {
        if (!text.empty()) {
            m_items.push_back({text, color});
        }
    }

    void ListBox::insertItem(int index, const std::string& text) {
        index = std::clamp(index, 0, itemCount());
        m_items.insert(m_items.begin() + index, ListItem{text, std::nullopt});
    }

    void ListBox::removeItem(int index) {
        if (index >= 0 && index < itemCount()) {
            m_items.erase(m_items.begin() + index);
        }
    }

    int ListBox::indexOf(const std::string& text) const {
        for (int i = 0; i < itemCount(); ++i) {
            if (m_items[static_cast<std::size_t>(i)].text == text) {
                return i;
            }
        }
        return -1;
    }

    void ListBox::setTopIndex(int index) {
        if (index < 0) {
            index = 0;
        }
        if (index < itemCount() || itemCount() == 0) {
            m_top = itemCount() == 0 ? 0 : index;
        }
    }

    void ListBox::scrollUp() {
        if (m_top > 0) {
            --m_top;
        }
    }

    void ListBox::scrollDown() {
        if (m_top + visibleRows() <= itemCount() - 1) {
            ++m_top;
        }
    }

    void ListBox::syncScrollBar(ScrollBar& scrollBar) {
        if (m_barCount != itemCount()) {
            scrollBar.setRange(itemCount(), visibleRows());
            m_barCount = itemCount();
            m_barPosition = -1;
        }
        if (m_barPosition != -1 && scrollBar.position() != m_barPosition) {
            m_top = scrollBar.position();
        } else if (scrollBar.position() != m_top) {
            scrollBar.setPosition(m_top);
        }
        m_barPosition = scrollBar.position();
    }

    void ListBox::onMouseDown(UiContext& context, MouseButton button, core::Point local) {
        (void)context;
        (void)local;
        if (button == MouseButton::Left) {
            m_pressed = true;
        }
    }

    void ListBox::onMouseUp(UiContext& context, MouseButton button, core::Point local) {
        const bool pressedHere = m_pressed;
        if (button == MouseButton::Left) {
            m_pressed = false;
        }
        const int row = rowAt(local);
        if (row < 0 || (button == MouseButton::Left && !pressedHere)) {
            return;
        }
        if (button == MouseButton::Left) {
            m_selected = m_toggleSelect && m_selected == m_top + row ? -1 : m_top + row;
            raise(context, "Select", m_top + row);
        } else {
            raise(context, "RightClick", m_top + row);
        }
    }

    void ListBox::onCaptureReleased(UiContext& context) {
        (void)context;
        m_pressed = false;
    }

    void ListBox::onDoubleClick(UiContext& context, core::Point local) {
        const int row = rowAt(local);
        if (row >= 0) {
            raise(context, "DoubleClick", m_top + row);
        }
    }

    void ListBox::onMouseWheel(UiContext& context, int steps) {
        (void)context;
        if (!m_wheel) {
            return;
        }
        for (int i = 0; i < std::abs(steps); ++i) {
            steps > 0 ? scrollUp() : scrollDown();
        }
    }

    void ListBox::onKeyDown(UiContext& context, const KeyEvent& key) {
        switch (key.key) {
            case Key::Up: // move the hover row up, or scroll up at the top
                if (m_hoverRow > 0) {
                    --m_hoverRow;
                } else {
                    scrollUp();
                }
                break;
            case Key::Down: // move the hover row down, or scroll down at the bottom
                if (m_hoverRow + 1 < visibleRows()) {
                    ++m_hoverRow;
                } else {
                    scrollDown();
                }
                break;
            case Key::Enter: // choose the hovered row (raises Select)
                if (m_hoverRow >= 0 && m_top + m_hoverRow < itemCount()) {
                    raise(context, "Select", m_top + m_hoverRow);
                }
                break;
            case Key::PageUp: // scroll one row up
                scrollUp();
                break;
            case Key::PageDown: // scroll one row down
                scrollDown();
                break;
            default: // other keys are ignored
                break;
        }
    }

    void ListBox::draw(DrawContext& context, core::Point screenPosition) {
        gfx::Canvas& canvas = context.canvas;
        const bool hovered = context.hoveredId == id();
        m_hoverRow = hovered ? rowAt(context.mouse - screenPosition) : -1;

        if ((hovered || m_alwaysFrame) && m_focusFrame) {
            // frame sticks out 2 left and 4 above; 4 wider and higher; pen 2 pixels wide
            canvas.drawRectOutline({screenPosition.x - 2, screenPosition.y - 4, bounds().width + 4, bounds().height + 4}, m_frameColor, 2);
        }
        if (m_showHoverBar && m_hoverRow >= 0) {
            // the bar starts 2 pixels above the row
            canvas.fillRectAlpha(
                {screenPosition.x, screenPosition.y - 2 + m_rowHeight * m_hoverRow, bounds().width - m_hoverBarTrim.x, m_rowHeight - m_hoverBarTrim.y},
                m_highlightColor
            );
        }
        if (m_showSelection && m_selected >= m_top && m_selected < m_top + visibleRows() && m_selected < itemCount()) {
            // the bar starts 2 pixels above the row
            canvas.fillRectAlpha({screenPosition.x, screenPosition.y - 2 + (m_selected - m_top) * m_rowHeight, bounds().width, m_rowHeight}, m_highlightColor);
        }

        const gfx::TextEffect effect = m_textShadow ? gfx::TextEffect::Shadow : gfx::TextEffect::Plain;
        for (int row = 0; row < visibleRows() && m_top + row < itemCount(); ++row) {
            const ListItem& item = m_items[static_cast<std::size_t>(m_top + row)];
            int lift = 0;
            int maxWidth = 512; // text wrap width in pixels when WrapColumns is off
            std::string text = item.text;
            if (m_wrapColumns > 0) {
                maxWidth = m_wrapColumns * gfx::TextRenderer::charWidth(effect);
                if (static_cast<int>(item.text.size()) > m_wrapColumns) {
                    lift = 4; // a wrapped (two-line) text is lifted 4 pixels
                }
                text = wrapTextLines(item.text, static_cast<std::size_t>(m_wrapColumns));
            }
            // text starts 2 pixels right of the list; the last argument 10 is the line height
            canvas.drawText(
                {screenPosition.x + 2 + m_textOffset.x, screenPosition.y + m_rowHeight * row + m_textOffset.y - lift},
                text,
                {item.color.value_or(m_fontColor), effect, std::nullopt},
                maxWidth,
                10
            );
        }
    }

}
