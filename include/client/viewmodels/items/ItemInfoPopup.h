#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_ITEMS_ITEM_INFO_POPUP_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_ITEMS_ITEM_INFO_POPUP_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "core/Rect.h"
#include "protocol/ItemInstance.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief Popup that describes the item under the mouse, with an optional worn-item comparison panel.
    /// @details Original: ItemInfoPopup.xml, TSe_PopInfoForm; the second panel is TEquipCompareForm.ShowCompare.
    class ItemInfoPopup {
        public:
            /// @brief Loads the popup panels and sets their painters.
            /// @param services Game services.
            /// @param screen Screen the popup is loaded into.
            /// @param status Player status (the Bottles' contents).
            ItemInfoPopup(
                GameServices& services,
                ui::Screen& screen,
                const PlayerStatus& status
            );

            /// @brief Shows what an item is beside its box, with what is worn instead when given.
            /// @param item The item to describe.
            /// @param itemBox Screen box of the item's cell.
            /// @param worn The worn item in its place, for the compare panel.
            void show(const protocol::ItemInstance& item, const core::Rect& itemBox, const std::optional<protocol::ItemInstance>& worn = std::nullopt);

            /// @brief Shows the description of an item lying on a shelf or display case of a house.
            /// @details Original: ShowThingItemInfo. The box lies 35 pixels beyond the 32 of its icon and the description is cut at the line width.
            /// @param item The item to describe.
            /// @param at Top-left corner of the item on the screen.
            void showStallItem(const protocol::ItemInstance& item, core::Point at);

            /// @brief Hides both panels.
            void hide();

            /// @brief The element id of the popup, for a form that wants it drawn over windows raised above popups (the item window).
            /// @return The panel's element id.
            int id() const { return m_panel.id(); }

        private:
            /// @brief Draws the lines, below a red header when there is one.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @param text Lines to draw.
            /// @param header Header text, or null for none.
            void paint(gfx::Canvas& canvas, core::Point at, const std::vector<std::string>& text, const char* header) const;

            /// @brief Puts the popup over every form again.
            /// @details Original: ShowCtrl makes the popup the active form.
            void raise();

            GameServices& m_services;
            ui::Screen& m_screen;
            const PlayerStatus& m_status;
            ui::Element& m_panel;
            ui::Element& m_comparePanel;
            const ui::Spot& m_lineSpot;
            const ui::Spot& m_headerSpot;
            std::vector<std::string> m_text;
            std::vector<std::string> m_compareText;
            protocol::ItemInstance m_shown;
            std::int32_t m_shownHp = 0;
            std::int32_t m_shownSp = 0;
            core::Rect m_shownBox;
    };

}

#endif
