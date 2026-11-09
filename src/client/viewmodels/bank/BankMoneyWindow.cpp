#include "client/viewmodels/bank/BankMoneyWindow.h"

#include "gfx/TextStyle.h"
#include "protocol/BankRequests.h"
#include "ui/DrawingArea.h"

#include <cstdio>
#include <utility>

namespace client {

    namespace {

        constexpr std::int32_t kMostGold = 9999999; // most gold the bank or the player can hold (seven digits)

        /// @brief Loads BankMoneyWindow.xml and returns its root window.
        /// @param services Game services (layout and load context).
        /// @param screen Screen to load into.
        /// @return The "BankMoney" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("BankMoneyWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("BankMoney");
        }

        /// @brief Formats a number right-aligned in seven characters.
        /// @details Original: Format('%7d').
        /// @param value Number to format.
        /// @return The padded text.
        std::string sevenWide(std::int32_t value) {
            char text[16]; // buffer size, ample for a 7-wide int
            std::snprintf(text, sizeof text, "%7d", static_cast<int>(value));
            return text;
        }

    }

    BankMoneyWindow::BankMoneyWindow(
        GameServices& services,
        ui::Screen& screen,
        const PlayerStatus& status,
        const std::int32_t& bankGold,
        PlayerMessages& messages
    )
        : m_services(services),
          m_screen(screen),
          m_status(status),
          m_bankGold(bankGold),
          m_messages(messages),
          m_window(loadWindow(services, screen)),
          m_withdrawAmount(screen.get<ui::TextBox>("BankMoneyWithdrawAmount")),
          m_depositAmount(screen.get<ui::TextBox>("BankMoneyDepositAmount")),
          m_textSpot(screen.get<ui::Spot>("BankMoneyText")) {
        screen.get<ui::DrawingArea>("BankMoneyPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onBankMoneyWithdraw", [this](const ui::EventArgs&) { withdraw(); });
        screen.on("onBankMoneyDeposit", [this](const ui::EventArgs&) { deposit(); });
        screen.on("onBankMoneyClose", [this](const ui::EventArgs&) { close(); });
        m_withdrawAmount.setText("0");
        m_depositAmount.setText("0");
    }

    void BankMoneyWindow::show(std::function<void()> onClosed) {
        m_onClosed = std::move(onClosed);
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void BankMoneyWindow::close() {
        m_window.setVisible(false);
        m_withdrawAmount.setText("0");
        m_depositAmount.setText("0");
        const std::function<void()> handler = std::move(m_onClosed);
        m_onClosed = nullptr;
        if (handler) {
            handler();
        }
    }

    std::int32_t BankMoneyWindow::amountOf(const std::string& text) {
        // at most 7 digits (9999999 is the largest amount)
        if (text.empty() || text.size() > 7 || text.find_first_not_of("0123456789") != std::string::npos) {
            return 0;
        }
        const std::int32_t amount = std::stoi(text);
        return amount > 0 && amount <= kMostGold ? amount : 0;
    }

    void BankMoneyWindow::withdraw() {
        if (m_withdrawPending || m_depositPending || m_status.gold >= kMostGold) {
            return;
        }
        const std::int32_t amount = amountOf(m_withdrawAmount.text());
        if (m_bankGold < amount) {
            m_messages.show("Unnable to withdraw, not enough money in your account", 2000); // message display time in ms
            return;
        }
        if (amount <= 0) {
            return;
        }
        m_withdrawPending = true;
        m_services.connection.send(protocol::bankGoldWithdrawRequest(amount));
    }

    void BankMoneyWindow::deposit() {
        if (m_withdrawPending || m_depositPending || m_bankGold >= kMostGold) {
            return;
        }
        const std::int32_t amount = amountOf(m_depositAmount.text());
        if (m_status.gold < amount) {
            m_messages.show("Unable to deposit, not enough money carried with you", 2000); // message display time in ms
            return;
        }
        if (amount <= 0) {
            return;
        }
        m_depositPending = true;
        m_services.connection.send(protocol::bankGoldDepositRequest(amount));
    }

    void BankMoneyWindow::done(bool deposit) {
        if (deposit) {
            m_depositPending = false;
        } else {
            m_withdrawPending = false;
        }
    }

    void BankMoneyWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        // bank gold at (80,70), carried gold at (280,70) in the window; text box 100x16
        canvas.drawText({at.x + 80, at.y + 70}, sevenWide(m_bankGold), style, 100, 16);
        canvas.drawText({at.x + 280, at.y + 70}, sevenWide(static_cast<std::int32_t>(m_status.gold)), style, 100, 16);
    }

}
