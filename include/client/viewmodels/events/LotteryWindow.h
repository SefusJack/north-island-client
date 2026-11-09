#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_LOTTERY_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_LOTTERY_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/events/LotteryPrizesWindow.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "protocol/LotteryLists.h"
#include "ui/DrawingArea.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <random>
#include <vector>

namespace client {

    /// @brief The online draw window: three draws a day with a spinning prize picture.
    /// @details Original: LotteryWindow.xml, TLa_LotteryOLForm; the prize is requested with 104/1 after 2.8 s.
    class LotteryWindow {
        public:
            /// @brief Loads the layout, builds the prize list window and wires events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param inventory Own inventory, checked for a free cell.
            /// @param messages Sink for short player messages.
            /// @param info Item info popup used by the prize list.
            /// @param send Sends a packet to the server.
            LotteryWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                std::function<void(const core::Bytes&)> send
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the window with the question mark again.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the window.
            void close() { m_window.setVisible(false); }

            /// @brief Takes the draws used and the prize list.
            /// @details Original: RecvLotteryInfo (104/1/1).
            /// @param info Draw info from the server.
            void infoReceived(const protocol::LotteryInfo& info);

            /// @brief Takes the draws used and shows the won prize.
            /// @details Original: RecvLotteryResult (104/1/2).
            /// @param result Draw result from the server.
            void resultReceived(const protocol::LotteryResult& result);

        private:
            /// @brief Starts a draw when one is left and the bag has room.
            /// @details Original: LotteryClick.
            void draw();

            /// @brief Tells whether the bag has an empty cell.
            /// @return True when a cell is free.
            bool hasFreeCell() const;

            /// @brief Shows a random prize that is not the one shown.
            void showRandomPrize();

            /// @brief Shows the icon of a prize.
            /// @param number Place in the prize list, from 1.
            void showPrize(int number);

            /// @brief Paints the draws left and runs the animation.
            /// @param canvas Target canvas.
            /// @param at Top left of the page area on the canvas.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the question mark, the prize icon or an empty box.
            /// @param canvas Target canvas.
            /// @param at Top left of the prize area on the canvas.
            void paintPrize(gfx::Canvas& canvas, core::Point at);

            /// @brief Steps the draw animation and asks the server for the prize at its end.
            /// @details Original: DrawControl's animation.
            /// @param nowMs Current time in milliseconds.
            void animate(std::uint64_t nowMs);

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            PlayerMessages& m_messages;
            std::function<void(const core::Bytes&)> m_send;
            ui::Window& m_window;
            ui::DrawingArea& m_prizeArea;
            const ui::Spot& m_drawsLeftSpot;
            const ui::Spot& m_textColorSpot;
            int m_drawsUsed = 0;
            std::vector<protocol::LotteryPrize> m_prizes;
            LotteryPrizesWindow m_prizesWindow;
            bool m_drawing = false;
            std::uint64_t m_nowMs = 0;
            std::uint64_t m_drawStartMs = 0;
            int m_phase = 0;
            int m_shown = 0;
            bool m_prizeShown = false;
            gfx::PictureId m_shownIcon;
            gfx::PictureId m_unknownIcon;
            std::mt19937 m_random{std::random_device{}()};
    };

}

#endif
