#ifndef NORTH_ISLAND_UI_COMBO_BOX_H
#define NORTH_ISLAND_UI_COMBO_BOX_H

#include "ui/TextBox.h"

#include <cstdint>
#include <vector>

namespace ui {

    /// @brief A text box with a drop-down button and a short list of earlier entries below it.
    /// @details The list closes when the combo box loses the keyboard. Attributes: DropButton, DropButtonOffset (from the top right corner), ListFontColor, ListHighlightColor, MaxItems. Events besides TextBox's: Select (an entry was picked), ItemRightClick.
    class ComboBox : public TextBox {
        public:
            /// @brief Creates the combo box and declares its Select and ItemRightClick events.
            ComboBox();

            /// @brief Reads the DropButton, DropButtonOffset, ListFontColor, ListHighlightColor and MaxItems attributes; others go to TextBox.
            /// @param name Attribute name.
            /// @param value Attribute value text.
            /// @param context Load context for parsing.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Resolves the drop button picture and its state height.
            /// @param context Load context for picture lookup.
            void finishLoading(const LoadContext& context) override;

            /// @brief Tells whether a point hits the drop button or the text box.
            /// @param local Point in element coordinates.
            /// @param pictures Picture library.
            /// @return True when the point takes the mouse.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief Tells whether a point hits the open drop-down list.
            /// @param local Point in element coordinates.
            /// @return True when the list is open and holds the point.
            bool hitTestOverlay(core::Point local) const override;

            /// @brief Presses the drop button, or passes the press to the text box.
            /// @param context UI context.
            /// @param button Pressed mouse button.
            /// @param local Point in element coordinates.
            void onMouseDown(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Toggles the list on a button release, or picks the entry under the mouse (Select on left, ItemRightClick on other buttons).
            /// @param context UI context used to raise events.
            /// @param button Released mouse button.
            /// @param local Point in element coordinates.
            void onMouseUp(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Scrolls the open list.
            /// @param context UI context (unused).
            /// @param steps Wheel steps, positive up.
            void onMouseWheel(UiContext& context, int steps) override;

            /// @brief Gives the list entries.
            /// @return The entries, newest first.
            const std::vector<std::string>& items() const { return m_items; }

            /// @brief Replaces the entries, cut to MaxItems, and scrolls to the top.
            /// @param items New entries.
            void setItems(std::vector<std::string> items);

            /// @brief Removes one entry.
            /// @param index Entry index; out of range does nothing.
            void removeItem(int index);

            /// @brief Moves text to the top of the list, adding it and dropping the oldest beyond MaxItems.
            /// @param text Entry text.
            void addToHistory(const std::string& text);

        protected:
            /// @brief Draws the text box and the drop button, and closes the list when focus is lost.
            /// @param context Draw context.
            /// @param screenPosition Top left of the element on screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

            /// @brief Draws the open drop-down list above other elements.
            /// @param context Draw context.
            /// @param screenPosition Top left of the element on screen.
            void drawOverlay(DrawContext& context, core::Point screenPosition) override;

        private:
            static constexpr int kRowHeight = 20; // height of one drop-down row, px
            static constexpr int kVisibleRows = 3; // rows shown before the list scrolls

            /// @brief Gives the drop button rectangle.
            /// @return The rectangle in element coordinates.
            core::Rect buttonArea() const;

            /// @brief Gives the open list's box: just below the field, one row per entry (three at most).
            /// @return The rectangle in element coordinates.
            core::Rect listArea() const;

            /// @brief Finds the list entry under a point.
            /// @param local Point in element coordinates.
            /// @return The entry index, or -1 for none or a closed list.
            int rowAt(core::Point local) const;

            /// @brief Opens or closes the list; does nothing when there are no entries.
            void toggle();

            std::vector<std::string> m_items;
            std::string m_buttonName = "Btn_ArrowDn_2";
            gfx::PictureId m_button;
            core::Point m_buttonOffset{0, 0};
            int m_buttonWidth = 0;
            int m_buttonHeight = 0;
            core::Color m_listFontColor = core::Color::fromRgb565(0x0841); // default list font colour as RGB565, about #080808
            core::Color m_listHighlight{255, 255, 0, 255}; // default highlight colour #FFFF00, opaque
            std::size_t m_maxItems = 5; // default MaxItems
            bool m_open = false;
            bool m_buttonPressed = false;
            int m_top = 0;
    };

}

#endif
