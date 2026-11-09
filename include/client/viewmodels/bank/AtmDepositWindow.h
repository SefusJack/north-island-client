#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_ATM_DEPOSIT_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_ATM_DEPOSIT_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief The ATM's deposit and withdrawal form: the gold in the ATM and carried, an amount put in or taken out.
    /// @details Original: AtmDepositWindow.xml, TATMDepositForm; deposit is 29/3, withdrawal 29/4.
    class AtmDepositWindow {
        public:
            static constexpr std::int64_t kMostAtmGold = 400000000; // most gold the ATM account may hold (400 million)
            static constexpr std::int64_t kMostCarried = 9999999; // most gold a character may carry (9,999,999)

            /// @brief Loads the form and wires its events.
            /// @param services Game services.
            /// @param screen Screen the form is loaded into.
            /// @param status The player's status (carried gold).
            /// @param atmGold Gold in the ATM.
            /// @param messages Short messages to the player.
            AtmDepositWindow(
                GameServices& services,
                ui::Screen& screen,
                const PlayerStatus& status,
                const std::uint32_t& atmGold,
                PlayerMessages& messages
            );

            /// @brief Tells whether the form is showing.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form with both amounts at 0.
            /// @details Original: ShowCtrl.
            /// @param onClosed Called when the form closes (HideCtrl asks for the next event result).
            void show(std::function<void()> onClosed);

            /// @brief Hides the form and calls the close handler.
            /// @details Original: HideCtrl.
            void close();

        private:
            /// @brief Checks the deposit amount and sends the deposit request.
            /// @details Original: ATMDeposit; request 29/3.
            void deposit();

            /// @brief Checks the withdrawal amount and sends the withdrawal request.
            /// @details Original: ATMWithdraw; request 29/4.
            void withdraw();

            /// @brief Reads the amount of a box from its digits.
            /// @param text The box text.
            /// @return The number, -1 when there are no digits.
            static std::int64_t amountOf(const std::string& text);

            /// @brief Checks that an amount is there and not zero, telling the player if not.
            /// @param amount Amount from amountOf.
            /// @return False when refused.
            bool hasAmount(std::int64_t amount);

            /// @brief Paints the gold in the ATM and the gold carried.
            /// @param canvas Target canvas.
            /// @param at Top-left of the painting area.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const PlayerStatus& m_status;
            const std::uint32_t& m_atmGold;
            PlayerMessages& m_messages;
            ui::Window& m_window;
            ui::TextBox& m_withdrawAmount;
            ui::TextBox& m_depositAmount;
            const ui::Spot& m_textSpot;
            std::function<void()> m_onClosed;
    };

}

#endif
