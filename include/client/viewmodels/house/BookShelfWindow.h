#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_BOOK_SHELF_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_BOOK_SHELF_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/BookShelves.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief The Bookcase and Desk windows of the player's house (BookShelfWindow.xml): ten places for blueprints each.
    /// @details Original: TH_Menu.pas TTH_UseBookCaseForm and TTH_UseBookDeskForm, the pieces 39054 and 39020. Blueprints dropped on a place go in (5/9, 5/11), a right click on one takes it back (5/10, 5/12); the answers (5/13, 5/24) change BookShelves.
    class BookShelfWindow {
        public:
            /// @brief Callbacks into the rest of the client that the shelves need.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
            };

            /// @brief Loads the window layouts and wires the close and take events.
            /// @param services Game services (layouts, item records).
            /// @param screen Screen the windows are loaded into.
            /// @param shelves The stored blueprints of the Bookcase and the Desk.
            /// @param inventory The player's bag.
            /// @param messages Player message line for refusals.
            /// @param info Item info popup shown under the mouse.
            /// @param hooks Callbacks into the rest of the client.
            BookShelfWindow(
                GameServices& services,
                ui::Screen& screen,
                const BookShelves& shelves,
                const Inventory& inventory,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                Hooks hooks
            );

            /// @brief Shows the Bookcase or the Desk window.
            /// @details Original: ShowCtrl, from the owner's click on the Bookcase (desk false) or the Desk.
            /// @param desk True for the Desk, false for the Bookcase.
            void open(bool desk);

            /// @brief Puts a blueprint from the bag onto a place (a full place sends it to the first empty one); refuses non-blueprints and a full shelf.
            /// @details Original: PutBookCaseItem and PutBookDeskItem, after the drop's check for a blueprint (Se_EquipUI HandleItemDrop, FSlotKind 14 and 25).
            /// @param desk True for the Desk, false for the Bookcase.
            /// @param bagSlot Bag slot of the dropped item.
            /// @param place Shelf place it was dropped on.
            void drop(bool desk, int bagSlot, int place);

            /// @brief Per frame: closes the forms out of the house, repaints the places and shows the info of the one under the mouse.
            /// @details Original: DrawControl; out of the house the forms go (HL_AllFormManager hides them).
            void update();

        private:
            /// @brief One shelf window with its icon grid.
            struct Shelf {
                ui::Window& window;
                ui::IconGrid& cells;
            };

            /// @brief Takes the blueprint of a place back to the bag (right click); refuses when the bag is full.
            /// @details Original: TakeBookCaseItem and TakeBookDeskItem.
            /// @param desk True for the Desk, false for the Bookcase.
            /// @param place Shelf place.
            void take(bool desk, int place);

            /// @brief Refills the icon cells of a shelf and shows the info of the item under the mouse.
            /// @details Original: SetBookCaseItem and SetBookDeskItem: the icon, no hint; the slot kinds 14 and 25 show no count.
            /// @param desk True for the Desk, false for the Bookcase.
            void paint(bool desk);

            /// @brief Gives the shelf window of the Desk or the Bookcase.
            /// @param desk True for the Desk, false for the Bookcase.
            /// @return The shelf.
            Shelf& shelf(bool desk) { return desk ? m_desk : m_case; }

            GameServices& m_services;
            ui::Screen& m_screen;
            const BookShelves& m_shelves;
            const Inventory& m_inventory;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            Hooks m_hooks;
            Shelf m_case;
            Shelf m_desk;
    };

}

#endif
