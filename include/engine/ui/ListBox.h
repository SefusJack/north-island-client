#ifndef NORTH_ISLAND_UI_LIST_BOX_H
#define NORTH_ISLAND_UI_LIST_BOX_H

#include "ui/Element.h"
#include "ui/ListItem.h"

#include <vector>

namespace ui {

    class ScrollBar;

    /// @brief A list of text rows with a hover bar, optional selection highlight, scrolling and a linked ScrollBar.
    /// @details Original: Tjo_SelectText FWheelDisabled, FTextOffsetX / Y, FAlwaysShowFrame, WrapTextLines, GetListHoverIndex.
    class ListBox : public Element {
        public:
            /// @brief Creates a list with the Select, DoubleClick and RightClick events and the hand cursor.
            ListBox();

            /// @brief Reads the ListBox attributes (RowHeight, FontColor, HighlightColor, FrameColor, ShowSelection, ShowHoverBar, ...).
            /// @param name The attribute name.
            /// @param value The attribute text.
            /// @param context The load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Whether the point lies in the list's rectangle.
            /// @param local The point relative to the list.
            /// @param pictures The picture library (unused).
            /// @return True when inside.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief Remembers that a left press began on this list.
            /// @param context The UI context.
            /// @param button The button.
            /// @param local The point relative to the list.
            void onMouseDown(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Chooses the row under the mouse (Select for the left button, RightClick for the right).
            /// @param context The UI context.
            /// @param button The button.
            /// @param local The point relative to the list.
            void onMouseUp(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Forgets the press when it was released elsewhere.
            /// @param context The UI context.
            void onCaptureReleased(UiContext& context) override;

            /// @brief Raises DoubleClick with the row index.
            /// @param context The UI context.
            /// @param local The point relative to the list.
            void onDoubleClick(UiContext& context, core::Point local) override;

            /// @brief Scrolls the list by the wheel steps unless Wheel is off.
            /// @param context The UI context.
            /// @param steps The wheel steps (positive up).
            void onMouseWheel(UiContext& context, int steps) override;

            /// @brief Moves the hover row or scrolls with Up, Down, PageUp and PageDown; Enter chooses the hovered row.
            /// @param context The UI context.
            /// @param key The key event.
            void onKeyDown(UiContext& context, const KeyEvent& key) override;

            /// @brief Removes every row and resets the selection and scroll position.
            void clear();

            /// @brief Appends a row (empty text is ignored).
            /// @param text The row text.
            /// @param color The row colour (nullopt: the list's FontColor).
            void addItem(const std::string& text, std::optional<core::Color> color = std::nullopt);

            /// @brief Inserts a row.
            /// @param index The position (clamped to the list).
            /// @param text The row text.
            void insertItem(int index, const std::string& text);

            /// @brief Removes a row if the index is valid.
            /// @param index The row index.
            void removeItem(int index);

            /// @brief The number of rows.
            /// @return The row count.
            int itemCount() const { return static_cast<int>(m_items.size()); }

            /// @brief The text of a row.
            /// @param index The row index (must be valid).
            /// @return The row text.
            const std::string& itemText(int index) const { return m_items[static_cast<std::size_t>(index)].text; }

            /// @brief Finds a row by its text.
            /// @param text The text to look for.
            /// @return The first matching index, or -1.
            int indexOf(const std::string& text) const;

            /// @brief The selected row.
            /// @return The index, or -1 for none.
            int selectedIndex() const { return m_selected; }

            /// @brief The row the mouse was over when the list was last drawn.
            /// @details Original: GetListHoverIndex.
            /// @return The row index, or -1 for none.
            int hoverIndex() const { return m_hoverRow >= 0 ? m_top + m_hoverRow : -1; }

            /// @brief Selects a row.
            /// @param index The row index (-1 for none).
            void setSelectedIndex(int index) { m_selected = index; }

            /// @brief The first visible row.
            /// @return The index.
            int topIndex() const { return m_top; }

            /// @brief Scrolls so a row is the first visible one.
            /// @param index The row index (ignored when past the end).
            void setTopIndex(int index);

            /// @brief How many rows fit in the list (a last partly shown row counts when a whole row of space remains).
            /// @return The row count.
            int visibleRows() const;

            /// @brief Scrolls one row up.
            void scrollUp();

            /// @brief Scrolls one row down.
            void scrollDown();

            /// @brief Turns the hover bar on or off.
            /// @param show True to show the bar.
            void setShowHoverBar(bool show) { m_showHoverBar = show; }

            /// @brief Turns the focus frame on or off.
            /// @details Original: FShowFocusFrame.
            /// @param show True to show the frame while the mouse is over the list.
            void setFocusFrame(bool show) { m_focusFrame = show; }

            /// @brief Shows the focus frame without the mouse over the list.
            /// @details Original: FAlwaysShowFrame.
            /// @param show True to always show the frame.
            void setAlwaysFrame(bool show) { m_alwaysFrame = show; }

            /// @brief Sets the row height.
            /// @param height The height in pixels (at least 1).
            void setRowHeight(int height) { m_rowHeight = height > 0 ? height : 1; }

            /// @brief The name of the ScrollBar element this list drives.
            /// @return The scroll bar name ("" for none).
            const std::string& scrollBarName() const { return m_scrollBarName; }

            /// @brief Keeps the list and its scroll bar showing the same rows.
            /// @param scrollBar The scroll bar to synchronise with.
            void syncScrollBar(ScrollBar& scrollBar);

        protected:
            /// @brief Draws the focus frame, hover and selection bars and the rows.
            /// @param context The draw context.
            /// @param screenPosition The list's top left on the screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            /// @brief The visible row under a point.
            /// @param local The point relative to the list.
            /// @return The row (0 is the first visible one), or -1 for none.
            int rowAt(core::Point local) const;

            std::vector<ListItem> m_items;
            int m_rowHeight = 20; // height of one row, px
            core::Color m_fontColor{255, 255, 255, 255}; // white
            core::Color m_highlightColor = core::Color::fromRgb(0x89BFF5); // light blue
            core::Color m_frameColor = core::Color::fromRgb(0x89BFF5); // light blue
            bool m_pressed = false;
            bool m_showSelection = false;
            bool m_showHoverBar = true;
            core::Point m_hoverBarTrim;
            bool m_focusFrame = true;
            bool m_alwaysFrame = false;
            bool m_toggleSelect = false;
            bool m_wheel = true;
            core::Point m_textOffset;
            bool m_textShadow = true;
            int m_wrapColumns = 0;
            std::string m_scrollBarName;

            int m_top = 0;
            int m_selected = -1;
            int m_hoverRow = -1;
            int m_barPosition = -1;
            int m_barCount = -1;
    };

}

#endif
