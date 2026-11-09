#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_NPC_SALE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_NPC_SALE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/IconGrid.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief Window for selling to an NPC: the bag with items to sell marked, what they bring, OK.
    /// @details Original: NpcSaleWindow.xml, TRe_SaleToNpcForm; OK sends 27/2.
    class NpcSaleWindow {
        public:
            /// @brief Loads the window layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param inventory The player's inventory.
            /// @param marks The bag cells' marks (items to sell are marked).
            /// @param status The player's status (gold).
            /// @param messages Player message line for refusals.
            /// @param bagChanged Called when the bag cells' marks changed.
            NpcSaleWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                BagMarks& marks,
                const PlayerStatus& status,
                PlayerMessages& messages,
                std::function<void()> bagChanged
            );

            /// @brief Whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the sale form for goods or equipment.
            /// @details Original: SetSaleKind and ShowCtrl (27/3 goods, 27/4 equipment).
            /// @param goods True for goods, false for equipment.
            /// @param onClosed Called when the form goes away (HideCtrl).
            void show(bool goods, std::function<void()> onClosed);

            /// @brief Shows how the sale went.
            /// @details Original: server packet 27/2 (0 is fine).
            /// @param result The server's result code.
            void sold(int result);

            /// @brief Redraws the bag after it changed.
            /// @details Original: RefreshGridItems.
            void refresh();

        private:
            /// @brief Marks or unmarks a bag item for sale, after the refusal checks.
            /// @details Original: ToggleSellItem.
            /// @param slot Bag slot clicked.
            void toggle(int slot);

            /// @brief Unmarks a bag item.
            /// @details Original: UnmarkSellItem.
            /// @param slot Bag slot.
            void unmark(int slot);

            /// @brief Sends the marked items to the server.
            /// @details Original: SellMarkedItems (27/2).
            void sell();

            /// @brief Clears all sale marks.
            /// @details Original: ClearSaleMarks.
            void clearMarks();

            /// @brief Hides the form, clears the marks and runs the closed handler.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Draws the sum the marked items bring and the gold carried.
            /// @param canvas Canvas to draw on.
            /// @param at Top-left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            BagMarks& m_marks;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            std::function<void()> m_bagChanged;
            ui::Window& m_window;
            ui::IconGrid& m_bag;
            ui::ImageButton& m_ok;
            ui::ImageButton& m_close;
            bool m_goods = true;
            std::function<void()> m_onClosed;
    };

}

#endif
