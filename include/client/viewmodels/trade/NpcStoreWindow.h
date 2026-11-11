#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_NPC_STORE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_NPC_STORE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/menus/CountBox.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/BlockList.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace client {

    /// @brief Window of an NPC's store, opened by a map event, with a ten place cart, the total and Buy.
    /// @details Original: NpcStoreWindow.xml, TRe_NpcStoreForm; Buy sends 27/1. An equipment store (kind 1) puts each item bought
    /// into a place of its own, a goods store (kind 2) stacks up to 50 and asks how many on Ctrl. Closing it, by a button or by
    /// buying, answers the event.
    class NpcStoreWindow {
        public:
            /// @brief The store kinds.
            enum class Kind { Equipment = 1, Goods = 2 };

            /// @brief Loads the window layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param inventory The player's inventory (worn discount card).
            /// @param status The player's status (gold).
            /// @param messages Player message line for refusals and results.
            /// @param centreMessages Centre message line for the discount notice.
            /// @param countBox Count box that asks how many to buy.
            NpcStoreWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                const PlayerStatus& status,
                PlayerMessages& messages,
                PlayerMessages& centreMessages,
                CountBox& countBox
            );

            /// @brief Whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Fills the store list and shows the window with an empty cart.
            /// @details Original: SetNpcStoreKind, AddNpcStoreItemRow for each item and ShowCtrl.
            /// @param kind Equipment or goods store.
            /// @param itemIds Ids of the items for sale.
            /// @param onClosed Called when the form goes away (TRe_NpcStoreForm.HideCtrl).
            void show(Kind kind, const std::vector<int>& itemIds, std::function<void()> onClosed);

            /// @brief Shows how the purchase went.
            /// @details Original: server packet 27/1 (0 fine, else what went wrong).
            /// @param result The server's result code.
            void bought(int result);

        private:
            /// @brief One cart place: the store row it holds and the count.
            struct CartLine {
                int row = -1;
                int count = 0;
            };

            /// @brief Puts a clicked store row into the cart, asking the count in a goods store on Ctrl.
            /// @details Original: HandleStoreListClick.
            /// @param row Store list row.
            /// @param askCount True to ask how many (Ctrl held).
            void pick(int row, bool askCount);

            /// @brief Adds an item to the cart.
            /// @details Original: AddToNpcCart.
            /// @param row Store list row.
            /// @param count How many.
            void addToCart(int row, int count);

            /// @brief Removes a cart place.
            /// @details Original: RemoveCartItem.
            /// @param cell Cart cell (1..10).
            void takeBack(int cell);

            /// @brief Sends the cart to the server if the gold does.
            /// @details Original: BuyCartItems (27/1).
            void buy();

            /// @brief Hides the form and runs the closed handler.
            void close();

            /// @brief Redraws the cart cells.
            void refresh();

            /// @brief Draws the cart total and the gold carried.
            /// @param canvas Canvas to draw on.
            /// @param at Top-left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief The price of everything in the cart.
            /// @return The total.
            std::int32_t total() const;

            static constexpr int kCartSlots = 10; // shopping cart slots, 1-based

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            PlayerMessages& m_centreMessages;
            CountBox& m_countBox;
            ui::Window& m_window;
            ui::BlockList& m_list;
            ui::IconGrid& m_cart;
            Kind m_kind = Kind::Goods;
            std::vector<int> m_itemIds;
            std::array<CartLine, kCartSlots + 1> m_lines{};
            std::function<void()> m_onClosed;
    };

}

#endif
