#include "ui/BlockList.h"

#include "gfx/TextStyle.h"
#include "ui/DrawContext.h"
#include "ui/UiContext.h"

#include <algorithm>

namespace ui {

    namespace {

        constexpr int kCharWidth = 8; // px width of one text character
        constexpr int kLineHeight = 15; // px height of a text line
        constexpr std::size_t kLongestText = 66; // longest block text shown, characters
        constexpr core::Point kCountOffset{2, 20}; // px offset of the item count text from the block's top left

        /// @brief Places a block of a given size inside an area along one axis.
        /// @param at Start of the area.
        /// @param size Size of the block.
        /// @param area Size of the area.
        /// @param align Start, End or Centre placement.
        /// @return The start coordinate of the block.
        int aligned(int at, int size, int area, BlockList::Align align) {
            switch (align) {
                case BlockList::Align::End: return at + area - size; // block flush with the end of the area
                case BlockList::Align::Centre: return area > size ? at + (area - size) / 2 : at; // block centred in the area, or at its start when too big
                case BlockList::Align::Start: break; // block at the start of the area
            }
            return at;
        }

    }

    BlockList::BlockList() {
        declareEvent("MouseDown");
        declareEvent("MouseDrag");
        declareEvent("MouseUp");
        declareEvent("Click");
        declareEvent("RightClick");
    }

    void BlockList::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "RowHeight") {
            m_rowHeight = std::max(1, LoadContext::parseInt(value));
        } else if (name == "RowSpacing") {
            m_rowSpacing = LoadContext::parseInt(value);
        } else if (name == "HighlightColor") {
            m_highlightColor = LoadContext::parseColor(value);
        } else if (name == "HoverAlpha") {
            m_hoverAlpha = LoadContext::parseInt(value);
        } else if (name == "SelectionAlpha") {
            m_selectionAlpha = LoadContext::parseInt(value);
        } else if (name == "ShowHoverBar") {
            m_showHoverBar = LoadContext::parseBool(value);
        } else if (name == "FontColor") {
            m_fontColor = LoadContext::parseColor(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    bool BlockList::hitTest(core::Point local, const gfx::PictureLibrary& pictures) const {
        (void)pictures;
        return core::Rect{0, 0, bounds().width, bounds().height}.contains(local);
    }

    int BlockList::visibleRows() const {
        const int pitch = m_rowHeight + m_rowSpacing;
        int rows = bounds().height / pitch;
        if (bounds().height - pitch * rows >= m_rowHeight) {
            ++rows;
        }
        return rows;
    }

    int BlockList::rowAt(core::Point local) const {
        const int pitch = m_rowHeight + m_rowSpacing;
        if (local.y < 0) {
            return -1;
        }
        const int row = local.y / pitch;
        if ((row + 1) * pitch - local.y <= m_rowSpacing || row >= visibleRows() || m_topIndex + row >= rowCount()) {
            return -1;
        }
        return row;
    }

    void BlockList::onMouseDown(UiContext& context, MouseButton button, core::Point local) {
        if (button == MouseButton::Left) {
            const int row = rowAt(local);
            m_lastMouse = local;
            raise(context, "MouseDown", row >= 0 ? m_topIndex + row : -1, local);
        }
    }

    void BlockList::onMouseDrag(UiContext& context, core::Point local) {
        m_lastMouse = local;
        raise(context, "MouseDrag", -1, local);
    }

    void BlockList::onCaptureReleased(UiContext& context) { raise(context, "MouseUp", -1, m_lastMouse); }

    void BlockList::onMouseUp(UiContext& context, MouseButton button, core::Point local) {
        const int row = rowAt(local);
        if (button == MouseButton::Left) {
            raise(context, "MouseUp", row >= 0 ? m_topIndex + row : -1, local);
        }
        if (button == MouseButton::Left && row >= 0) {
            if (!context.releaseOfDoubleClick) {
                raise(context, "Click", m_topIndex + row, local);
            }
        } else if (button == MouseButton::Right && row >= 0) {
            raise(context, "RightClick", m_topIndex + row, local);
        }
    }

    void BlockList::scrollUp() {
        if (m_topIndex - 1 >= 0) {
            --m_topIndex;
        }
    }

    void BlockList::scrollDown() {
        if (rowCount() - 1 >= m_topIndex + visibleRows()) {
            ++m_topIndex;
        }
    }

    void BlockList::pageUp() {
        if (rowCount() < visibleRows()) {
            m_topIndex = 0;
            return;
        }
        m_topIndex = std::max(0, m_topIndex - visibleRows());
    }

    void BlockList::pageDown() {
        if (rowCount() < visibleRows()) {
            m_topIndex = 0;
            return;
        }
        m_topIndex += visibleRows();
        if (rowCount() < m_topIndex + visibleRows()) {
            m_topIndex = rowCount() - visibleRows();
        }
    }

    void BlockList::drawPicture(gfx::Canvas& canvas, const PictureBlock& block, core::Point rowOrigin) const {
        if (!block.picture.isValid()) {
            return;
        }
        const core::Point at{
            aligned(rowOrigin.x + block.offset.x, block.size.width, block.area.width, block.horizontal),
            aligned(rowOrigin.y + block.offset.y, block.size.height, block.area.height, block.vertical)
        };
        if (block.source) {
            canvas.drawPictureScaled(block.picture, {at.x, at.y, block.size.width, block.size.height}, block.source);
        } else {
            canvas.drawPicture(block.picture, at);
        }
        if (block.count) {
            canvas.drawText(
                {at.x + kCountOffset.x, at.y + kCountOffset.y},
                std::to_string(*block.count),
                {m_fontColor, gfx::TextEffect::Plain, std::nullopt},
                bounds().width,
                kLineHeight
            );
        }
    }

    void BlockList::drawText(gfx::Canvas& canvas, const TextBlock& block, core::Point rowOrigin) const {
        const std::string text = block.text.substr(0, kLongestText);
        if (text.empty()) {
            return;
        }
        const int width = static_cast<int>(text.size()) * kCharWidth;
        const bool fits = width < block.area.width;
        core::Point at{rowOrigin.x + block.offset.x, rowOrigin.y + block.offset.y};
        if (fits) {
            at.x = aligned(at.x, width, block.area.width, block.horizontal);
            if (block.vertical == Align::End) {
                at.y += block.area.height - kLineHeight;
            } else if (block.vertical == Align::Centre && block.area.height > kLineHeight) {
                at.y += (block.area.height - kLineHeight) / 2;
            }
        }
        canvas.drawText(at, text, {block.color, gfx::TextEffect::Plain, std::nullopt}, block.area.width, kLineHeight);
    }

    void BlockList::draw(DrawContext& context, core::Point screenPosition) {
        gfx::Canvas& canvas = context.canvas;
        const int pitch = m_rowHeight + m_rowSpacing;
        const int hover = context.hoveredId == id() ? rowAt(context.mouse - screenPosition) : -1;
        core::Color bar = m_highlightColor;
        bar.a = static_cast<std::uint8_t>(m_hoverAlpha);
        if (hover >= 0 && m_showHoverBar) {
            canvas.fillRectAlpha(
                {screenPosition.x, screenPosition.y - 2 + pitch * hover, bounds().width, m_rowHeight},
                bar
            ); // 2 = pixels the rows start above the element top
        }
        const int last = std::max(0, std::min(visibleRows(), rowCount() - m_topIndex));
        for (int index = 0; index < last; ++index) {
            if (m_rows[static_cast<std::size_t>(m_topIndex + index)].checked) {
                canvas.fillRectAlpha(
                    {screenPosition.x, screenPosition.y - 2 + pitch * index, bounds().width, m_rowHeight},
                    bar
                ); // 2 = pixels the rows start above the element top
            }
        }
        if (m_selectionAlpha >= 0 && m_selected >= m_topIndex && m_selected < m_topIndex + last) {
            core::Color selection = m_highlightColor;
            selection.a = static_cast<std::uint8_t>(m_selectionAlpha);
            canvas.fillRectAlpha(
                {screenPosition.x, screenPosition.y - 2 + pitch * (m_selected - m_topIndex), bounds().width, m_rowHeight},
                selection
            ); // 2 = pixels the rows start above the element top
        }
        for (int index = 0; index < last; ++index) {
            const Row& row = m_rows[static_cast<std::size_t>(m_topIndex + index)];
            const core::Point origin{screenPosition.x, screenPosition.y - 2 + index * pitch}; // 2 = pixels the rows start above the element top
            const std::size_t blocks = std::max(row.pictures.size(), row.texts.size());
            for (std::size_t block = 0; block < blocks; ++block) {
                if (block < row.pictures.size()) {
                    drawPicture(canvas, row.pictures[block], origin);
                }
                if (block < row.texts.size()) {
                    drawText(canvas, row.texts[block], origin);
                }
            }
        }
    }

}
