#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_STALL_CASE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_STALL_CASE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HouseInterior.h"
#include "client/models/house/StallCaseChange.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/DrawingArea.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The owner's view of one of his stall (display) cases: what it holds in stacks of at most 50, its price, Setup to change the price, a right click on a stack to take it back.
    /// @details Original: StallCaseWindow.xml, HL_Menu.pas THL_StallCaseForm; bag items dropped on it add to the case (62/58).
    class StallCaseWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
                std::function<void(int, std::int32_t)> openReprice;
                std::function<void()> closePrice;
            };

            /// @brief Loads the form and wires its events.
            /// @param services Game services (layouts, items).
            /// @param screen Screen the form is loaded into.
            /// @param house The house interior (stall cases).
            /// @param inventory The player's bag.
            /// @param marks Bag cell marks.
            /// @param messages Sink for player notices.
            /// @param hooks Owner callbacks.
            StallCaseWindow(
                GameServices& services,
                ui::Screen& screen,
                const HouseInterior& house,
                const Inventory& inventory,
                const BagMarks& marks,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form for a known case.
            /// @param furnIndex Index of the furniture piece.
            /// @param caseNo Number of the case.
            /// @param itemId Item the case holds.
            /// @param price Price of the case.
            /// @details Original: TStallInFurItem.HandleClick for the owner.
            void open(int furnIndex, int caseNo, int itemId, std::int32_t price);

            /// @brief Shows the form for a piece, keeping the case it had (a new item with a price is sent to be put in and the answer names the case).
            /// @param furnIndex Index of the furniture piece.
            void openForPiece(int furnIndex);

            /// @brief Hides the form (and the price form for a case) and forgets the case.
            /// @details Original: HideCtrl, ResetStallCase.
            void close();

            /// @brief Adds a bag item to the case, or (setPrice) puts it in a new case at the given price.
            /// @param bagSlot Bag slot of the item.
            /// @param setPrice True to put the item in a new case at price.
            /// @param price Price of the new case.
            /// @details Original: PutItemInStallCase and PutStallCaseItem (62/58).
            void putItem(int bagSlot, bool setPrice, std::int32_t price);

            /// @brief Sends a new price for the case when it differs from the current one.
            /// @param price The new price.
            /// @details Original: SendStallCasePrice.
            void sendPrice(std::int32_t price);

            /// @brief Follows a change of the own case: new price is said, an emptied case closes the form.
            /// @param change The case before and after.
            /// @details Original: RecvStallCaseUpdate option 1 for the own player.
            void changed(const StallCaseChange& change);

        private:
            /// @brief One of the five boxes of the case: the item and how many (stacks of 50 of the case's count).
            struct Stack {
                int itemId = 0;
                int count = 0;
            };

            /// @brief Computes the stack shown in a box.
            /// @param box Box 1..5.
            /// @return The item and count, or an empty stack.
            /// @details Original: RefreshStallCaseSlots.
            Stack stackAt(int box) const;

            /// @brief Shows the form in front.
            /// @details Original: ShowCtrl.
            void show();

            /// @brief Asks the server to take the stack of a box back to the bag.
            /// @param box Box 1..5.
            /// @details Original: TakeStallCaseItems.
            void take(int box);

            /// @brief Draws the item name, the price and the stacks; closes the form out of the house.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @details Original: DrawControl (CloseHouseUseForms at a map change).
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const HouseInterior& m_house;
            const Inventory& m_inventory;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::IconGrid& m_cells;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_priceSpot;
            int m_furnIndex = 0;
            int m_caseNo = 0;
            int m_itemId = 0;
            std::int32_t m_price = 0;
    };

}

#endif
