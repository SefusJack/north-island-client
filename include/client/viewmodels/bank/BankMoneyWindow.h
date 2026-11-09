#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_BANK_MONEY_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_BANK_MONEY_WINDOW_H

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

    /// @brief Bank money account window: gold in the account and carried, withdraw or deposit an amount.
    /// @details Original: BankMoneyWindow.xml, TMoneyAccountForm; packets 29/1 (withdraw) and 29/2 (deposit).
    class BankMoneyWindow {
        public:
            /// @brief Builds the window and wires its text boxes and buttons.
            /// @param services Game services.
            /// @param screen Screen the window lives on.
            /// @param status Player status (carried gold).
            /// @param bankGold Gold in the bank account.
            /// @param messages Player message sink.
            BankMoneyWindow(
                GameServices& services,
                ui::Screen& screen,
                const PlayerStatus& status,
                const std::int32_t& bankGold,
                PlayerMessages& messages
            );

            /// @brief Tells whether the window is visible.
            /// @return True while open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the window.
            /// @details Original: ShowCtrl; onClosed runs CloseMoneyAccount.
            /// @param onClosed Called when the window closes.
            void show(std::function<void()> onClosed);

            /// @brief The server finished a withdrawal or deposit.
            /// @details Original: server 29/7 (withdrawal) and 29/8 (deposit).
            /// @param deposit True for a deposit, false for a withdrawal.
            void done(bool deposit);

        private:
            /// @brief Asks the server to take the entered amount out of the account.
            /// @details Original: WithdrawMoney (29/1).
            void withdraw();

            /// @brief Asks the server to put the entered amount into the account.
            /// @details Original: DepositMoney (29/2).
            void deposit();

            /// @brief Closes the window and clears the amounts.
            /// @details Original: HideCtrl and ResetMoneyAmounts.
            void close();

            /// @brief Parses an amount text.
            /// @details Original: ParseMoneyAmount.
            /// @param text Text typed by the player.
            /// @return The amount (1..9999999), else 0.
            static std::int32_t amountOf(const std::string& text);

            /// @brief Paints the gold figures over the window.
            /// @param canvas Target canvas.
            /// @param at Window origin.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const PlayerStatus& m_status;
            const std::int32_t& m_bankGold;
            PlayerMessages& m_messages;
            ui::Window& m_window;
            ui::TextBox& m_withdrawAmount;
            ui::TextBox& m_depositAmount;
            const ui::Spot& m_textSpot;
            bool m_withdrawPending = false;
            bool m_depositPending = false;
            std::function<void()> m_onClosed;
    };

}

#endif
