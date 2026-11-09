#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_SALE_MENU_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_SALE_MENU_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/items/Inventory.h"
#include "client/models/items/ItemStore.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>

namespace client {

    /// @brief Manager window of the sale net: gold moves, bag/store item choice and sale requests.
    /// @details Original: SaleMenuWindow.xml, TCY_SaleMenu, 23/64 opens it; 26/3 and 26/4 move gold; 23/40 and 23/41 put and
    /// take back items; 23/44 requests the sale list and 23/42 the manager page; 23/65 is the pending-request reply.
    class SaleMenuWindow {
        public:
            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param bag Player's bag.
            /// @param store The ten places of the sale store.
            /// @param status Player status (level, gold).
            /// @param saleGold Gold held in the sale account.
            /// @param requestPending Shared pending-request marker (0 none, 1 send, 2 manage, 3 net get).
            /// @param messages Player message line.
            SaleMenuWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& bag,
                const ItemStore& store,
                const PlayerStatus& status,
                const std::int32_t& saleGold,
                std::uint8_t& requestPending,
                PlayerMessages& messages
            );

            /// @brief Tells whether the window is visible.
            /// @return True when shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the window.
            /// @details Original: ShowCtrl; onClosed is CloseSaleMenu (the sale net's list goes and the event goes on).
            /// @param onClosed Called when the window is closed.
            void show(std::function<void()> onClosed);

            /// @brief Redraws the bag and store cells after a change.
            /// @details Original: SyncSaleBagImages, SyncSaleItemImages.
            void refresh();

        private:
            static constexpr int kBagSlots = 50; // player bag slots, 1-based
            static constexpr int kStoreSlots = 10; // stall slots, 1-based

            /// @brief Toggles the chosen mark of a bag cell, refusing items the sale rejects.
            /// @details Original: TCY_SaleBody.HandleMouseDown.
            /// @param slot Bag slot (1-based).
            void pressBag(int slot);

            /// @brief Toggles the chosen mark of a store place.
            /// @details Original: TCY_SaleItem.HandleMouseDown.
            /// @param slot Store place (1-based).
            void pressStore(int slot);

            /// @brief Sends the chosen bag items to the sale store.
            /// @details Original: PutItemsForSale (23/40).
            void put();

            /// @brief Takes the chosen store items back to the bag.
            /// @details Original: TakeBackSaleItems (23/41).
            void takeBack();

            /// @brief Requests the sale manager page.
            /// @details Original: RequestSaleManage (23/42).
            void manage();

            /// @brief Requests the sale list.
            /// @details Original: RequestSendSale (23/44).
            void send();

            /// @brief Moves the entered gold into the sale account.
            /// @details Original: DepositSaleMoney (26/3).
            void deposit();

            /// @brief Moves the entered gold out of the sale account.
            /// @details Original: WithdrawSaleMoney (26/4).
            void withdraw();

            /// @brief Steps an amount box up or down by one within its limits.
            /// @param box Amount box to change.
            /// @param step Signed step (+1 or -1).
            /// @param most Largest allowed value.
            void changeAmount(ui::TextBox& box, int step, std::int32_t most);

            /// @brief Checks the level 20 rule and that no request is pending.
            /// @return True when a request may be sent.
            bool levelAllowed();

            /// @brief Reads a digits-only amount box.
            /// @param box Box to read.
            /// @return The number, or 0 when empty or invalid.
            static std::int32_t amountOf(const ui::TextBox& box);

            /// @brief Hides the window and runs the closed handler once.
            void close();

            /// @brief Paints the carried gold and the sale account gold.
            /// @param canvas Target canvas.
            /// @param at Window origin on the canvas.
            void paintMoney(gfx::Canvas& canvas, core::Point at);

            /// @brief Tints the chosen bag cells and store places.
            /// @param canvas Target canvas.
            /// @param at Window origin on the canvas.
            void paintChosen(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_bag;
            const ItemStore& m_store;
            const PlayerStatus& m_status;
            const std::int32_t& m_saleGold;
            std::uint8_t& m_requestPending;
            PlayerMessages& m_messages;
            ui::Window& m_window;
            ui::IconGrid& m_storeGrid;
            ui::IconGrid& m_bagGrid;
            ui::TextBox& m_amount;
            ui::TextBox& m_count;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_chosenSpot;
            std::array<bool, kBagSlots + 1> m_bagChosen{};
            std::array<bool, kStoreSlots + 1> m_storeChosen{};
            std::function<void()> m_onClosed;
    };

}

#endif
