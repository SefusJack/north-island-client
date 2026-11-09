#include "client/viewmodels/bank/AtmDepositWindow.h"

#include "gfx/TextStyle.h"
#include "protocol/BankRequests.h"
#include "ui/DrawingArea.h"

#include <utility>

namespace client {

    namespace {

        constexpr int kShortMs = 1300; // error message display time, ms

        /// @brief Loads the deposit form's layout and gives its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "AtmDeposit" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("AtmDepositWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("AtmDeposit");
        }

    }

    AtmDepositWindow::AtmDepositWindow(
        GameServices& services,
        ui::Screen& screen,
        const PlayerStatus& status,
        const std::uint32_t& atmGold,
        PlayerMessages& messages
    )
        : m_services(services),
          m_screen(screen),
          m_status(status),
          m_atmGold(atmGold),
          m_messages(messages),
          m_window(loadWindow(services, screen)),
          m_withdrawAmount(screen.get<ui::TextBox>("AtmDepositWithdrawAmount")),
          m_depositAmount(screen.get<ui::TextBox>("AtmDepositDepositAmount")),
          m_textSpot(screen.get<ui::Spot>("AtmDepositText")) {
        screen.get<ui::DrawingArea>("AtmDepositPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onAtmDepositDeposit", [this](const ui::EventArgs&) { deposit(); });
        screen.on("onAtmDepositWithdraw", [this](const ui::EventArgs&) { withdraw(); });
        screen.on("onAtmDepositClose", [this](const ui::EventArgs&) { close(); });
    }

    void AtmDepositWindow::show(std::function<void()> onClosed) {
        m_onClosed = std::move(onClosed);
        m_withdrawAmount.setText("0");
        m_depositAmount.setText("0");
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void AtmDepositWindow::close() {
        if (!m_window.isVisible()) {
            return;
        }
        m_window.setVisible(false);
        const std::function<void()> handler = std::move(m_onClosed);
        m_onClosed = nullptr;
        if (handler) {
            handler();
        }
    }

    std::int64_t AtmDepositWindow::amountOf(const std::string& text) {
        std::int64_t amount = 0;
        bool any = false;
        for (const char digit : text) {
            if (digit >= '0' && digit <= '9') {
                amount = amount * 10 + (digit - '0'); // decimal digit accumulation
                any = true;
            }
        }
        return any ? amount : -1; // -1: no digits
    }

    bool AtmDepositWindow::hasAmount(std::int64_t amount) {
        if (amount < 0) {
            m_messages.show("Money can't be void", kShortMs);
            return false;
        }
        if (amount == 0) {
            m_messages.show("Money can't be zero", kShortMs);
            return false;
        }
        return true;
    }

    void AtmDepositWindow::deposit() {
        const std::int64_t amount = amountOf(m_depositAmount.text());
        if (!hasAmount(amount)) {
            return;
        }
        if (m_status.gold - amount < 0) {
            m_messages.show("Insufficient money", kShortMs);
            return;
        }
        if (static_cast<std::int64_t>(m_atmGold) + amount > kMostAtmGold) {
            m_messages.show("Exceeded the upper limit of ATM : 4 hundred million", kShortMs);
            return;
        }
        m_services.connection.send(protocol::atmDepositRequest(static_cast<std::int32_t>(amount)));
    }

    void AtmDepositWindow::withdraw() {
        const std::int64_t amount = amountOf(m_withdrawAmount.text());
        if (!hasAmount(amount)) {
            return;
        }
        if (static_cast<std::int64_t>(m_atmGold) - amount < 0) {
            m_messages.show("Insufficient money", kShortMs);
            return;
        }
        if (m_status.gold + amount > kMostCarried) {
            m_messages.show("Reached the cash limit amount", kShortMs);
            return;
        }
        m_services.connection.send(protocol::atmWithdrawRequest(static_cast<std::int32_t>(amount)));
    }

    void AtmDepositWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        // ATM gold at (104, 66) and carried gold at (270, 66) in the form; text box 100 wide, 16 high
        canvas.drawText({at.x + 104, at.y + 66}, std::to_string(m_atmGold), style, 100, 16);
        canvas.drawText({at.x + 270, at.y + 66}, std::to_string(m_status.gold), style, 100, 16);
    }

}
