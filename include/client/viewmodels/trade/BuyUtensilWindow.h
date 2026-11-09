#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_BUY_UTENSIL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_BUY_UTENSIL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/items/Inventory.h"
#include "client/models/items/OfferGrid.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/UtensilStallGoods.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>

namespace client {

    /// @brief Window showing someone's stall of large items (one to three pieces) to mark and buy.
    /// @details Original: BuyUtensilWindow.xml, TTH_BuyUtensilForm; Buy sends 23/68.
    class BuyUtensilWindow {
        public:
            /// @brief Loads the window layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param inventory The player's inventory (bag occupancy).
            /// @param status The player's status (gold).
            /// @param messages Player message line for refusals.
            /// @param info Item info popup shown over the hovered piece.
            /// @param sportActive Returns true while a sport event blocks buying.
            BuyUtensilWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                const PlayerStatus& status,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                std::function<bool()> sportActive
            );

            /// @brief Whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief The stall owner's id.
            /// @return The owner id of the shown goods.
            std::int32_t ownerId() const { return m_goods.ownerId; }

            /// @brief Shows the form for the received stall goods.
            /// @details Original: RecvUtensilStallList (the form for one, two or three pieces), then ShowCtrl.
            /// @param goods The stall's pieces.
            void show(const protocol::UtensilStallGoods& goods);

            /// @brief Closes the window and clears its pieces and marks.
            /// @details Original: CloseBuyUtensilForm / HideCtrl: ClearBuyUtensilForm.
            void close();

            /// @brief Describes the piece under the mouse.
            void update();

        private:
            /// @brief Clears the marks and takes the bag's occupied cells.
            /// @details Original: ResetBuyUtensilMarks, MarkBagOccupancy.
            void markBag();

            /// @brief Marks or unmarks a piece to buy, reserving a place in the bag.
            /// @details Original: ToggleBuyUtensilSlot.
            /// @param slot Piece slot (1..3).
            void toggle(int slot);

            /// @brief Asks the server to buy the marked pieces.
            /// @details Original: BuySelectedUtensils (23/68).
            void buy();

            /// @brief Draws the totals (marked prices and gold carried).
            void paint();

            /// @brief Bag cells a piece occupies.
            /// @param slot Piece slot (1..3).
            /// @return Width and height in cells.
            core::Size cellsOf(int slot) const;

            /// @brief Builds the icon cell of a piece.
            /// @param piece The stall piece.
            /// @param marked Whether the piece is marked (framed).
            /// @return The cell.
            ui::IconCell pieceCell(const protocol::UtensilStallGoods::Piece& piece, bool marked) const;

            static constexpr int kSlots = 3;

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            std::function<bool()> m_sportActive;
            ui::Window& m_window;
            const ui::Spot& m_memoSpot;
            protocol::UtensilStallGoods m_goods;
            // 10: rows of the bag's occupancy grid
            OfferGrid m_bag{10};
            std::array<bool, kSlots + 1> m_marked{};
            std::array<int, kSlots + 1> m_reserved{};
            int m_clickedSlot = 0;
    };

}

#endif
