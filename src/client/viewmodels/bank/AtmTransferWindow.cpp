#include "client/viewmodels/bank/AtmTransferWindow.h"

#include "gfx/TextStyle.h"
#include "protocol/BankRequests.h"
#include "ui/DrawingArea.h"

#include <utility>

namespace client {

    namespace {

        constexpr int kShortMs = 1300; // error message display time, ms
        constexpr int kOfflineMs = 2000; // "doesn't exist or off line" message display time, ms

        /// @brief Loads the transfer form's layout and gives its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "AtmTransfer" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("AtmTransferWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("AtmTransfer");
        }

    }

    AtmTransferWindow::AtmTransferWindow(
        GameServices& services,
        ui::Screen& screen,
        const std::uint32_t& atmGold,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_atmGold(atmGold),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_name(screen.get<ui::TextBox>("AtmTransferName")),
          m_amount(screen.get<ui::TextBox>("AtmTransferAmount")),
          m_textSpot(screen.get<ui::Spot>("AtmTransferText")) {
        screen.get<ui::DrawingArea>("AtmTransferPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onAtmTransferConfirm", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onAtmTransferList", [this](const ui::EventArgs&) { m_hooks.openList(); });
        screen.on("onAtmTransferClose", [this](const ui::EventArgs&) { close(); });
    }

    void AtmTransferWindow::show(std::function<void()> onClosed) {
        m_onClosed = std::move(onClosed);
        m_name.setText("");
        m_amount.setText("0");
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void AtmTransferWindow::close() {
        if (!m_window.isVisible()) {
            return;
        }
        m_window.setVisible(false);
        m_hooks.closeList();
        const std::function<void()> handler = std::move(m_onClosed);
        m_onClosed = nullptr;
        if (handler) {
            handler();
        }
    }

    void AtmTransferWindow::confirm() {
        const std::string text = m_amount.text();
        if (text.find_first_of("0123456789") == std::string::npos) {
            m_messages.show("Money can't be void", kShortMs);
            return;
        }
        std::int64_t amount = 0;
        for (const char digit : text) {
            if (digit >= '0' && digit <= '9') {
                amount = amount * 10 + (digit - '0'); // decimal digit accumulation
            }
        }
        if (amount == 0) {
            m_messages.show("Money can't be zero", kShortMs);
            return;
        }
        if (static_cast<std::int64_t>(m_atmGold) - amount < 0) {
            m_messages.show("Insufficient money", kShortMs);
            return;
        }
        const std::string name = m_name.text();
        const std::optional<std::int32_t> target = m_hooks.findPlayer(name);
        if (!target) {
            m_messages.show("Doesn't exist or off line", kOfflineMs);
            return;
        }
        const std::int32_t playerId = *target;
        const std::int32_t gold = static_cast<std::int32_t>(amount);
        m_hooks
            .ask("You will " + std::to_string(gold) + " Transfer your money to " + name + " account?", [this, playerId, gold] { send(playerId, gold); }, [] {});
    }

    void AtmTransferWindow::send(std::int32_t playerId, std::int32_t amount) {
        m_services.connection.send(protocol::atmTransferRequest(playerId, amount));
        close();
    }

    void AtmTransferWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        // ATM gold at (66, 85) in the form; text box 100 wide, 16 high
        canvas.drawText({at.x + 66, at.y + 85}, std::to_string(m_atmGold), style, 100, 16);
    }

}
