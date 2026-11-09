#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_PLEDGE_BOX_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_PLEDGE_BOX_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HouseBoxes.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief The Locker form of a house: twenty-five places for pledge items (item type 21).
    /// @details Original: PledgeBoxWindow.xml, HL_Menu.pas THL_PledgeBoxForm; bag items dropped on a place go in (62/52, option 7), a right click on a place takes the item back (option 8).
    class PledgeBoxWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
            };

            static constexpr int kLockerItem = 39015; // item 39015: Locker (house furniture, type 29)

            /// @brief Loads the form and wires its events.
            /// @param services Game services (layouts, items).
            /// @param screen Screen the form is loaded into.
            /// @param boxes What lies in the Locker's places.
            /// @param inventory The player's bag.
            /// @param marks Bag cell marks.
            /// @param messages Sink for player notices.
            /// @param info Item info popup.
            /// @param hooks Owner callbacks.
            PledgeBoxWindow(
                GameServices& services,
                ui::Screen& screen,
                const HouseBoxes& boxes,
                const Inventory& inventory,
                const BagMarks& marks,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the Locker form, pulled back onto the screen.
            /// @param piece Index of the Locker piece in the house.
            /// @details Original: ShowCtrl.
            void open(int piece);

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

            /// @brief Puts a bag item into a Locker place (pledge items only) and sends the request.
            /// @param bagSlot Bag slot of the dropped item.
            /// @param place Locker place the item was dropped on.
            /// @details Original: PutItemInPledgeBox (62/52, option 7); a drop on an occupied place goes to the first free place.
            void drop(int bagSlot, int place);

            /// @brief Shows what lies in the places and the info of the item under the mouse.
            /// @details Original: DrawControl.
            void update();

        private:
            /// @brief Asks the server to take the item of a place back to the bag.
            /// @param place Locker place (1-based).
            /// @details Original: TakeItemFromPledgeBox (62/52, option 8).
            void take(int place);

            GameServices& m_services;
            ui::Screen& m_screen;
            const HouseBoxes& m_boxes;
            const Inventory& m_inventory;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::IconGrid& m_cells;
            int m_piece = 0;
    };

}

#endif
