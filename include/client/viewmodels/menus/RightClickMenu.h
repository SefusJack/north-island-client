#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MENUS_RIGHT_CLICK_MENU_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MENUS_RIGHT_CLICK_MENU_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/rules/BagRequests.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Point.h"
#include "ui/Screen.h"
#include "ui/Element.h"

#include <functional>

namespace client {

    /// @brief Right-click menu on a bag item: "Put in" for HP/SP Bottles, "Store Firing" for the Store Tank.
    /// @details Original: RightClickMenu.xml, TRightClickForm; PutPotionsInBottle pours every potion of the Bottle's kind into it.
    class RightClickMenu {
        public:
            /// @brief Loads the menu layout and wires its buttons.
            /// @param services Game services (layouts, items, pictures).
            /// @param screen Screen the menu is loaded into.
            /// @param inventory Player inventory (bag items).
            /// @param status Player status (bottle fill levels).
            /// @param messages Sink for short player messages.
            /// @param requests Bag requests (item moves).
            /// @param openTank Called with the bag slot to open the Store Tank form.
            RightClickMenu(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                const PlayerStatus& status,
                PlayerMessages& messages,
                BagRequests& requests,
                std::function<void(int bagSlot)> openTank
            );

            /// @brief Opens the menu for the item in a bag slot, beside its cell; nothing opens for other items.
            /// @details Original: ShowCtrl.
            /// @param bagSlot Bag slot of the item.
            /// @param cell Top left corner of the item's cell on the screen.
            void open(int bagSlot, core::Point cell);

            /// @brief Hides the menu.
            void hide();

            /// @brief Hides the menu when the mouse has left its area.
            /// @details Original: DrawControl.
            void update();

            /// @brief Tells whether the menu is showing.
            /// @return True when the menu panel is visible.
            bool isOpen() const { return m_panel.isVisible(); }

        private:
            /// @brief Pours the bag's potions of the Bottle's kind into the Bottle.
            void putIn();

            /// @brief Opens the Store Tank form.
            /// @details Original: PutPotionsInBottle for the Store Tank.
            void storeFiring();

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            BagRequests& m_requests;
            std::function<void(int)> m_openTank;
            ui::Element& m_panel;
            int m_bottleSlot = 0;
            bool m_forTank = false;
    };

}

#endif
