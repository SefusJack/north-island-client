#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_POINT_EXCHANGE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_POINT_EXCHANGE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief ATM point exchange form: IM points are given for gold.
    /// @details Original: PointExchangeWindow.xml, MK_Menu.pas TExchange; opened by 23/200, OK sends 23/109, server answers 23/201.
    class PointExchangeWindow {
        public:
            /// @brief Callbacks into the rest of the game.
            struct Hooks {
                std::function<std::int32_t()> imPoints;
                std::function<std::int32_t()> gold;
                std::function<int()> mapId;
                std::function<bool()> inOwnHouse;
                std::function<void(const std::string&, std::function<void()>, std::function<void()>)> ask;
                std::function<void(const std::string&)> chatLine;
                std::function<void(std::int32_t)> setGold;
                std::function<void(std::int32_t)> setAtmGold;
                std::function<std::string(std::int32_t)> playerName;
            };

            static constexpr std::int64_t kMostCarried = 9999999; // most gold a character may carry (9,999,999)

            /// @brief Builds the window and wires its controls.
            /// @param services Game services.
            /// @param screen Screen the window lives on.
            /// @param messages Player message sink.
            /// @param hooks Callbacks into the player state.
            PointExchangeWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True while open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form with the gold a point is worth.
            /// @details Original: RecvExchangeRate (23/200).
            /// @param rate Gold per point.
            void open(std::int32_t rate);

            /// @brief The server answered with the gold now and what was gained.
            /// @details Original: RecvExchangeGold (23/201).
            /// @param gold Gold now carried.
            /// @param gained Gold gained by the exchange.
            void goldReceived(std::int32_t gold, std::int32_t gained);

            /// @brief Shows a fee message from the server.
            /// @details Original: RecvExchangeFeeMsg (23/202), kind 1 to 5.
            /// @param kind Message kind, 1 to 5.
            /// @param playerId Player the message is about.
            /// @param value Amount in the message.
            void feeMessage(int kind, std::int32_t playerId, std::int32_t value);

            /// @brief Hides the form and resets the number to 0.
            /// @details Original: HideCtrl.
            void hide();

        private:
            /// @brief Computes the gold the entered points make.
            /// @details Original: DrawControl's gold, points times rate, less the host's 1000 outside the free places.
            /// @return Gold amount.
            std::int64_t goldOfPoints() const;

            /// @brief Tells whether no host fee is taken here (Store ATM or own house).
            /// @return True when the fee is free.
            bool feeFree() const;

            /// @brief Asks the player to confirm the exchange.
            /// @details Original: ConfirmPointExchange.
            void confirm();

            /// @brief Sends the exchange request.
            /// @details Original: SendPointExchange (23/109).
            void send();

            /// @brief Paints the figures over the window.
            /// @param canvas Target canvas.
            /// @param at Window origin.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_points;
            const ui::Spot& m_rateSpot;
            const ui::Spot& m_feeMarkSpot;
            const ui::Spot& m_goldSpot;
            std::int32_t m_rate = 0;
    };

}

#endif
