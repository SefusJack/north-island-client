#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_ICE_BOX_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_ICE_BOX_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HouseBoxes.h"
#include "client/models/house/HouseInterior.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/IconGrid.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief The form of the house storage boxes: Fridge, Material Cabinet, Potion Cabinet and Medicine Case (ten places each).
    /// @details Original: IceBoxWindow.xml, HL_Menu.pas THL_IceBoxForm; bag items dropped on a place go in (62/52), a right click on a place takes the item back; the Fridge form also has the cooking button.
    class IceBoxWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<void(int piece)> cook;
            };

            /// @brief Loads the form and wires its events.
            /// @param services Game services (layouts, pictures, items).
            /// @param screen Screen the form is loaded into.
            /// @param house The house interior (furniture pieces).
            /// @param boxes What lies in each box's places.
            /// @param inventory The player's bag.
            /// @param marks Bag cell marks.
            /// @param messages Sink for player notices.
            /// @param info Item info popup.
            /// @param hooks Owner callbacks.
            IceBoxWindow(
                GameServices& services,
                ui::Screen& screen,
                const HouseInterior& house,
                const HouseBoxes& boxes,
                const Inventory& inventory,
                const BagMarks& marks,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                Hooks hooks
            );

            /// @brief Tells which box a furniture piece opens.
            /// @param itemId Furniture item id.
            /// @return 1 Fridge (39023), 2 Material Cabinet (39132), 3 Potion Cabinet (39157), 4 Medicine Case (39188), else 0.
            /// @details Original: the cases of ShowCtrl.
            static int boxOf(int itemId);

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form of the box a piece stands for.
            /// @param piece Index of the furniture piece in the house.
            /// @details Original: ShowCtrl (form kept where it was and pulled back onto the screen).
            void open(int piece);

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

            /// @brief Puts a bag item into a place of the box after the original's checks, and sends the request.
            /// @param bagSlot Bag slot of the dropped item.
            /// @param place Box place the item was dropped on.
            /// @details Original: PutItemInIceBox (62/52); a drop on an occupied place goes to the first free place.
            void drop(int bagSlot, int place);

            /// @brief Shows what lies in the places and the info of the item under the mouse.
            /// @details Original: DrawControl.
            void update();

        private:
            /// @brief Asks the server to take the item of a place back to the bag.
            /// @param place Box place (1-based).
            /// @details Original: TakeItemFromIceBox.
            void take(int place);

            /// @brief Tells whether the current box takes an item.
            /// @param itemId Item id.
            /// @return True when the box accepts it.
            /// @details Original: the item types, CanPutInIceBox, CanStoreInIceBox.
            bool accepts(int itemId) const;

            GameServices& m_services;
            ui::Screen& m_screen;
            const HouseInterior& m_house;
            const HouseBoxes& m_boxes;
            const Inventory& m_inventory;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::IconGrid& m_cells;
            ui::ImageButton& m_cooking;
            ui::ImageButton& m_close;
            ui::ImageButton& m_closeSmall;
            int m_piece = 0;
            int m_box = 1;
    };

}

#endif
