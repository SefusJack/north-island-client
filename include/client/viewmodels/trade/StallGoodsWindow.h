#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_STALL_GOODS_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_STALL_GOODS_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/menus/CountBox.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/StallGoods.h"
#include "ui/BlockList.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace client {

    /// @brief Window of someone's item stall: the items with what is left, a cart of five, the total and Buy.
    /// @details Original: StallGoodsWindow.xml, TRe_BuyItemStallForm; item list 23/33, Buy 23/33.
    class StallGoodsWindow {
        public:
            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param inventory Player's bag (room check when buying).
            /// @param status Player status (gold).
            /// @param messages Player message line.
            /// @param countBox Count entry box for Ctrl-picks.
            StallGoodsWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                const PlayerStatus& status,
                PlayerMessages& messages,
                CountBox& countBox
            );

            /// @brief Tells whether the window is visible.
            /// @return True when shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Gives the id of the stall's owner.
            /// @return Owner id of the shown stall.
            std::int32_t ownerId() const { return m_goods.ownerId; }

            /// @brief Opens a fresh form with the stall's items.
            /// @details Original: RecvStallItemList (23/33).
            /// @param goods Items of the stall.
            /// @param ownerName Owner's name.
            /// @param stallName Stall's name.
            void show(const protocol::StallGoods& goods, const std::string& ownerName, const std::string& stallName);

            /// @brief Hides the window.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief One cart place: which stall row it holds and how many.
            struct CartLine {
                int row = -1;
                int count = 0;
            };

            /// @brief Handles a click on a stall row: one more, or with Ctrl the count box.
            /// @details Original: HandleStallListClick.
            /// @param row Row index (0-based).
            /// @param askCount True to ask for the count.
            void pick(int row, bool askCount);

            /// @brief Moves items from a stall row into the cart (up to 50 per cart place).
            /// @details Original: AddStallItemToCart / FillBuyCartSlot.
            /// @param row Row index (0-based).
            /// @param count Number to add.
            /// @return True when the items were added.
            bool addToCart(int row, int count);

            /// @brief Returns a cart place to the stall's counts.
            /// @details Original: RemoveBuyCartItem.
            /// @param cell Cart place (1-based).
            void takeBack(int cell);

            /// @brief Checks room and gold, then sends the purchase and closes.
            /// @details Original: BuyStallCartItems (23/33).
            void buy();

            /// @brief Updates the row counts and the cart icons.
            /// @details Original: SyncStallItemCounts and the cart's icons.
            void refresh();

            /// @brief Paints the total, the gold and the owner/stall caption.
            /// @param canvas Target canvas.
            /// @param at Window origin on the canvas.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Sums the price of the cart.
            /// @return Total price in gold.
            std::int32_t total() const;

            static constexpr int kCartSlots = 5;

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            CountBox& m_countBox;
            ui::Window& m_window;
            ui::BlockList& m_list;
            ui::IconGrid& m_cart;
            const ui::Spot& m_ownerSpot;
            protocol::StallGoods m_goods;
            std::vector<int> m_left;
            std::array<CartLine, kCartSlots + 1> m_lines{};
            std::string m_ownerName;
            std::string m_stallName;
    };

}

#endif
