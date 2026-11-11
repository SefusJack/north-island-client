#ifndef NORTH_ISLAND_UI_BLOCK_LIST_H
#define NORTH_ISLAND_UI_BLOCK_LIST_H

#include "core/Color.h"
#include "core/Size.h"
#include "gfx/PictureLibrary.h"
#include "ui/Element.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace ui {

    /// @brief A list whose rows are made of pictures and texts.
    /// @details Original: TRe_ImgListItem. Each block sits in an area of its row, at its start, end or centre. A text that does not fit its area's width starts at the area's top left and wraps; texts are cut to 66 characters (string[66]). The row under the mouse gets a see-through highlight bar, and so does a checked row (FCheckMode: the view model checks and unchecks rows on Click). Attributes: RowHeight, RowSpacing, HighlightColor, HoverAlpha, ShowHoverBar (FShowHoverBar: no bar under the mouse), SelectionAlpha (FShowSelection with FSelAlphaBlend: the selected row gets a bar of that alpha after the hover bar), FontColor (the counts under pictures). Events: MouseDown (left button pressed on the list: the row), MouseDrag (mouse moves while held), MouseUp (comes up, wherever), Click and RightClick (let go over a row: index). The rows shown start at the top row (FTopIndex), which the view model moves; indexes count from the first row.
    class BlockList : public Element {
        public:
            /// @brief Placement of a block inside its area along one axis.
            enum class Align { Start, End, Centre };

            /// @brief A picture in a row, optionally with a count at its bottom left or a shrunk source part.
            struct PictureBlock {
                gfx::PictureId picture;
                core::Point offset;
                core::Size size{32, 32}; // default picture size in pixels
                core::Size area{32, 32}; // default area size in pixels
                Align horizontal = Align::Start;
                Align vertical = Align::Start;
                std::optional<int> count;
                std::optional<core::Rect> source;
            };

            /// @brief A text in a row, aligned inside its area.
            struct TextBlock {
                std::string text;
                core::Point offset;
                core::Size area;
                Align horizontal = Align::Start;
                Align vertical = Align::Start;
                core::Color color{8, 8, 8, 255}; // default text colour #080808, opaque
            };

            /// @brief One row of the list: its picture blocks, text blocks and checked flag.
            struct Row {
                std::vector<PictureBlock> pictures;
                std::vector<TextBlock> texts;
                bool checked = false;
            };

            /// @brief Creates the list and declares its mouse and click events.
            BlockList();

            /// @brief Reads the list attributes (RowHeight, RowSpacing, HighlightColor, HoverAlpha, SelectionAlpha, ShowHoverBar, FontColor); others go to Element.
            /// @param name Attribute name.
            /// @param value Attribute value text.
            /// @param context Load context for parsing.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Tells whether a point is inside the list.
            /// @param local Point in element coordinates.
            /// @param pictures Picture library (unused).
            /// @return True when the point is inside the element bounds.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief Raises MouseDown with the row under a left press.
            /// @param context UI context used to raise the event.
            /// @param button Pressed mouse button.
            /// @param local Point in element coordinates.
            void onMouseDown(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Raises MouseUp, Click or RightClick for the row under the release.
            /// @param context UI context used to raise events.
            /// @param button Released mouse button.
            /// @param local Point in element coordinates.
            void onMouseUp(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Raises MouseDrag while the button is held.
            /// @param context UI context used to raise the event.
            /// @param local Point in element coordinates.
            void onMouseDrag(UiContext& context, core::Point local) override;

            /// @brief Raises MouseUp when the button came up outside the list.
            /// @param context UI context used to raise the event.
            void onCaptureReleased(UiContext& context) override;

            /// @brief Removes all rows.
            void clear() { m_rows.clear(); }

            /// @brief Appends a row.
            /// @param row The row to add.
            void addRow(Row row) { m_rows.push_back(std::move(row)); }

            /// @brief Counts the rows.
            /// @return Number of rows.
            int rowCount() const { return static_cast<int>(m_rows.size()); }

            /// @brief Gives access to a row.
            /// @param index Row index, counted from the first row.
            /// @return The row.
            Row& row(int index) { return m_rows[static_cast<std::size_t>(index)]; }

            /// @brief Finds the row shown at a point of the list.
            /// @details Original: RenderWithChildren's FHoverRow. 0 is the top shown row; the spacing below a row is not a row.
            /// @param local Point in element coordinates.
            /// @return The shown row number, or -1 for none.
            int rowAt(core::Point local) const;

            /// @brief Gives the index of the top shown row.
            /// @return The top row index.
            int topIndex() const { return m_topIndex; }

            /// @brief Gives the selected row (FSelectedIndex); drawn only with a SelectionAlpha.
            /// @return The selected index, or -1 for none.
            int selected() const { return m_selected; }

            /// @brief Selects a row.
            /// @param index Row index, or -1 for none.
            void setSelected(int index) { m_selected = index; }

            /// @brief Gives the height of a row plus the spacing below it.
            /// @return The row pitch in pixels.
            int pitch() const { return m_rowHeight + m_rowSpacing; }

            /// @brief Moves the top shown row.
            /// @param index New top row index, clamped to 0 or more.
            void setTopIndex(int index) { m_topIndex = std::max(0, index); }

            /// @brief Scrolls up a row (original ScrollListUp).
            void scrollUp();

            /// @brief Scrolls down a row (original ScrollListDown).
            void scrollDown();

            /// @brief Scrolls up a page (original PageListUp).
            void pageUp();

            /// @brief Scrolls down a page (original PageListDown).
            void pageDown();

            /// @brief Counts the rows that fit, one more when the rest holds a whole row.
            /// @details Original: SetListBounds.
            /// @return The number of visible rows.
            int visibleRows() const;

        protected:
            /// @brief Draws the highlight bars and the visible rows.
            /// @param context Draw context.
            /// @param screenPosition Top left of the element on screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            /// @brief Draws a picture block of a row, with its count.
            /// @param canvas Canvas to draw on.
            /// @param block The picture block.
            /// @param rowOrigin Top left of the row on screen.
            void drawPicture(gfx::Canvas& canvas, const PictureBlock& block, core::Point rowOrigin) const;

            /// @brief Draws a text block of a row.
            /// @param canvas Canvas to draw on.
            /// @param block The text block.
            /// @param rowOrigin Top left of the row on screen.
            void drawText(gfx::Canvas& canvas, const TextBlock& block, core::Point rowOrigin) const;

            std::vector<Row> m_rows;
            int m_topIndex = 0;
            core::Point m_lastMouse;
            int m_rowHeight = 20; // default row height in pixels
            int m_rowSpacing = 0;
            core::Color m_highlightColor = core::Color::fromRgb(0x89BEF5); // default highlight colour #89BEF5
            int m_hoverAlpha = 255; // default bar alpha, opaque
            int m_selectionAlpha = -1;
            int m_selected = -1;
            bool m_showHoverBar = true;
            core::Color m_fontColor{8, 8, 8, 255}; // default count colour #080808, opaque
    };

}

#endif
