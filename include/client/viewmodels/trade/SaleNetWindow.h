#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_SALE_NET_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_SALE_NET_WINDOW_H

#include "client/services/GameServices.h"
#include "protocol/ItemInstance.h"
#include "protocol/SaleNetList.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief Window of what the sale net holds for the player: ten web-store items with checks and a gold amount to ask for.
    /// @details Original: SaleNetWindow.xml, TCY_SaleNetMenu, 23/63 opens it; Get sends the amount and checked places (23/43).
    class SaleNetWindow {
        public:
            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param requestPending Shared pending-request marker.
            SaleNetWindow(
                GameServices& services,
                ui::Screen& screen,
                std::uint8_t& requestPending
            );

            /// @brief Tells whether the window is visible.
            /// @return True when shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Fills the places from the server list and shows the form.
            /// @details Original: RecvSaleNetList (23/63): every place cleared, those of the list filled, the form shown.
            /// @param list Sale net list received from the server.
            void show(const protocol::SaleNetList& list);

            /// @brief Hides the window.
            /// @details Original: HideCtrl.
            void hide() { m_window.setVisible(false); }

        private:
            static constexpr int kPlaces = 10; // stall places, 1-based

            /// @brief Toggles the check of a place (not for an item on sale nor an empty place).
            /// @details Original: TCY_CheckImage.HandleMouseDown.
            /// @param place Place number (1-based).
            void pressCheck(int place);

            /// @brief Steps the amount box by one.
            /// @details Original: IncSaleNetAmount / DecSaleNetAmount.
            /// @param step Signed step (+1 or -1).
            void changeAmount(int step);

            /// @brief Sends the amount and checked places, then clears the checks and the amount.
            /// @details Original: RequestSaleNetGet (23/43).
            void get();

            /// @brief Rebuilds the item and check cells.
            void refresh();

            /// @brief Paints the sale net's gold value.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Window origin on the canvas.
            void paintValue(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the on-sale and competed-for marks.
            /// @details Original: RenderWithChildren's marks.
            /// @param canvas Target canvas.
            /// @param at Window origin on the canvas.
            void paintMarks(gfx::Canvas& canvas, core::Point at);

            /// @brief Reads the amount box.
            /// @return The number, or 0 when empty or invalid.
            std::int32_t amount() const;

            GameServices& m_services;
            ui::Screen& m_screen;
            std::uint8_t& m_requestPending;
            ui::Window& m_window;
            ui::IconGrid& m_items;
            ui::IconGrid& m_checks;
            ui::TextBox& m_amount;
            const ui::Spot& m_valueSpot;
            std::int32_t m_value = 0;
            std::array<protocol::ItemInstance, kPlaces + 1> m_places{};
            std::array<int, kPlaces + 1> m_state{};
    };

}

#endif
