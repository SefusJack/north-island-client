#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_ITEM_STALL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_ITEM_STALL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/ItemInstance.h"
#include "protocol/OwnStall.h"
#include "protocol/StallSale.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief Window for the player's own item stall: up to five bag items with prices, shop name and sign.
    /// @details Original: ItemStallWindow.xml, TRe_ItemStallForm; Set a Stall 23/30, End Stall 23/31, gold from sales 23/34.
    class ItemStallWindow {
        public:
            /// @brief Queries and actions the window needs from the rest of the client.
            struct Hooks {
                std::function<void()> bagChanged;
                std::function<int()> mapId;
                std::function<bool()> inTeam;
                std::function<bool()> keepsItemStall;
                std::function<bool()> keepsAnyStall;
                std::function<bool()> riding;
                std::function<void(const std::string& line)> chatLine;
                std::function<bool()> usesVehicle;
                std::function<bool()> fishing;
                std::function<bool()> remoteOn;
            };

            /// @brief Loads the window layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param inventory The player's inventory.
            /// @param marks The bag cells' marks (on sale, busy, hidden).
            /// @param messages Player message line for refusals.
            /// @param hooks Queries and actions supplied by the scene.
            ItemStallWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                BagMarks& marks,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the window when closed, hides it when open.
            /// @details Original: ToggleCtrlShown.
            void toggle();

            /// @brief Shows the window, with a fresh form unless the stall is kept.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the window.
            /// @details Original: HideCtrl.
            void hide() { m_window.setVisible(false); }

            /// @brief The server opened the stall: the prices are kept, the form goes away.
            /// @details Original: RecvOwnItemStall.
            /// @param stall The opened stall (unused).
            void stallOpened(const protocol::OwnStall& stall);

            /// @brief The stall was closed: the price boxes become editable again.
            /// @details Original: TPlayer.CloseItemStall.
            void stallClosed();

            /// @brief Shows a sale: chat line, cleared row, income.
            /// @details Original: RecvStallSale (23/34).
            /// @param sale The sale.
            void showSale(const protocol::StallSale& sale);

            /// @brief Re-reads the bag after it changed.
            /// @details Original: SyncStallSlots, RefreshGridItems.
            void refresh();

        private:
            /// @brief One stall row: the bag slot it comes from and a copy of the item.
            struct Row {
                int bagSlot = 0;
                protocol::ItemInstance item;
            };

            /// @brief Puts a bag item into the first free stall row, after the refusal checks.
            /// @details Original: AddBagItemToStall / PutBagItemInStall.
            /// @param bagSlot Bag slot clicked.
            void addFromBag(int bagSlot);

            /// @brief Removes the row that holds a bag slot.
            /// @details Original: RemoveBagItemFromStall, RemoveStallSlot.
            /// @param bagSlot Bag slot clicked.
            void removeByBagSlot(int bagSlot);

            /// @brief Removes a stall row and its bag mark.
            /// @param row Row number (1..5).
            void removeRow(int row);

            /// @brief Empties a stall row and resets its price.
            /// @param row Row number (1..5).
            void clearRow(int row);

            /// @brief Closes the form; an unkept stall is cleared first.
            /// @details Original: CloseItemStallForm.
            void close();

            /// @brief Checks the game's conditions and asks the server to open the stall.
            /// @details Original: StartItemStall (server opens it with 23/30).
            void keep();

            /// @brief Asks the server to end the stall.
            /// @details Original: StopItemStall (23/31).
            void revoke();

            /// @brief Checks the stall items, prices and shop name.
            /// @details Original: ValidateStallItems.
            /// @return True when the stall may be opened.
            bool validate();

            /// @brief Resets the form to an empty stall.
            /// @details Original: ResetItemStallForm.
            void reset();

            /// @brief Moves the focus to the next price box that has an item.
            /// @details Original: FocusNextRow.
            /// @param row The row to go on from.
            void focusNextRow(int row);

            /// @brief Shows the Set/End buttons that match the stall state.
            void layoutButtons();

            /// @brief Draws the earned gold and the stall sign.
            /// @param canvas Canvas to draw on.
            /// @param at Top-left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief The price box of a row.
            /// @param row Row number (1..5).
            /// @return The text box.
            ui::TextBox& price(int row);

            /// @brief The price typed for a row.
            /// @param row Row number (1..5).
            /// @return The price, 0 when empty.
            std::int32_t priceOf(int row);

            static constexpr int kRows = 5;

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            BagMarks& m_marks;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::IconGrid& m_bag;
            ui::IconGrid& m_rows;
            ui::TextBox& m_name;
            const ui::Spot& m_incomeSpot;
            const ui::Spot& m_signSpot;
            std::array<Row, kRows + 1> m_items{};
            int m_sign = 1;
            std::int32_t m_income = 0;
            std::string m_lastName;
            int m_lastSign = 0;
    };

}

#endif
