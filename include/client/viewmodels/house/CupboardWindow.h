#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_CUPBOARD_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_CUPBOARD_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HouseInterior.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/IconGrid.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief A cupboard window of a house (CupboardWindow.xml, TTH_UseCupboardForm): the Wardrobe's five places or the Cabinet's ten.
    /// @details Original: TH_Menu.pas TTH_UseCupboardForm. Bag items dropped on a place go in (62/41), a right click on one takes it back (62/42).
    class CupboardWindow {
        public:
            /// @brief Callbacks into the rest of the client that the cupboard needs.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
            };

            /// @brief Loads the window layout and wires the close and take events.
            /// @param services Game services (layouts, item records).
            /// @param screen Screen the window is loaded into.
            /// @param house The house interior with its cupboards.
            /// @param inventory The player's bag.
            /// @param marks Bag slot marks (locks).
            /// @param messages Player message line for refusals.
            /// @param info Item info popup shown under the mouse.
            /// @param hooks Callbacks into the rest of the client.
            CupboardWindow(
                GameServices& services,
                ui::Screen& screen,
                const HouseInterior& house,
                const Inventory& inventory,
                const BagMarks& marks,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                Hooks hooks
            );

            /// @brief Tells whether the window is shown.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form of the cupboard of a piece, in the piece's size, kept where it was and pulled back onto the screen.
            /// @details Original: ShowCtrl.
            /// @param furnIndex Furniture piece index of the cupboard.
            void open(int furnIndex);

            /// @brief Hides the window.
            void close() { m_window.setVisible(false); }

            /// @brief Handles a bag item let go on a place: the checks in the original's order, then the put request (62/41).
            /// @details Original: HandleItemDrop on a place of the form (FSlotKind 13) and PutCupboardItem.
            /// @param bagSlot Bag slot of the dropped item.
            /// @param place Cupboard place it was dropped on.
            void drop(int bagSlot, int place);

            /// @brief Per frame: closes the form out of the house, shows what lies in the places and the info of the item under the mouse.
            /// @details Original: DrawControl.
            void update();

        private:
            /// @brief Takes the item of a place back to the bag (right click, 62/42); refuses when the bag is full.
            /// @details Original: TakeCupboardItem.
            /// @param place Cupboard place.
            void take(int place);

            /// @brief Gives the icon grid of the open form.
            /// @return The five-place or the ten-place grid.
            ui::IconGrid& cells() const;

            GameServices& m_services;
            ui::Screen& m_screen;
            const HouseInterior& m_house;
            const Inventory& m_inventory;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::IconGrid& m_fiveCells;
            ui::IconGrid& m_tenCells;
            ui::ImageButton& m_close;
            const ui::Spot& m_wardrobeCloseSpot;
            const ui::Spot& m_cabinetCloseSpot;
            int m_furnIndex = 0;
            int m_kind = 1;
    };

}

#endif
