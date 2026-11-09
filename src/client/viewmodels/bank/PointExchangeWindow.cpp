#include "client/viewmodels/bank/PointExchangeWindow.h"

#include "gfx/TextStyle.h"
#include "protocol/ExchangeRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kStoreMap = 10080; // map 10080: Store (no host fee)
        constexpr int kOtherFreeMap = 10005; // map 10005: Test (no host fee)
        constexpr std::int64_t kHostFee = 1000; // gold fee for the ATM host, taken off the exchanged gold

        /// @brief Loads PointExchangeWindow.xml and returns its root window.
        /// @param services Game services (layout and load context).
        /// @param screen Screen to load into.
        /// @return The "PointExchange" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PointExchangeWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("PointExchange");
        }

    }

    PointExchangeWindow::PointExchangeWindow(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_points(screen.get<ui::TextBox>("ExchangePoints")),
          m_rateSpot(screen.get<ui::Spot>("ExchangeRate")),
          m_feeMarkSpot(screen.get<ui::Spot>("ExchangeFeeMark")),
          m_goldSpot(screen.get<ui::Spot>("ExchangeGold")) {
        screen.get<ui::DrawingArea>("ExchangePaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onExchangeOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onExchangeClose", [this](const ui::EventArgs&) { hide(); });
        m_points.setText("0");
    }

    void PointExchangeWindow::open(std::int32_t rate) {
        m_rate = rate;
        m_window.setVisible(true);
    }

    void PointExchangeWindow::hide() {
        m_window.setVisible(false);
        m_points.setText("0");
    }

    bool PointExchangeWindow::feeFree() const {
        const int map = m_hooks.mapId();
        return map == kOtherFreeMap || map == kStoreMap || m_hooks.inOwnHouse();
    }

    std::int64_t PointExchangeWindow::goldOfPoints() const {
        std::int64_t total = (m_points.text().empty() ? 0 : std::stoll(m_points.text())) * m_rate;
        if (!feeFree() && total != 0) {
            total -= kHostFee;
        }
        return total;
    }

    void PointExchangeWindow::confirm() {
        std::int64_t total = goldOfPoints();
        if (m_hooks.gold() + total > kMostCarried) {
            m_messages.show("Reached the cash limit amount", 1300); // message display time in ms
            const std::int64_t most = m_rate != 0 ? (kMostCarried - m_hooks.gold()) / m_rate : 0;
            m_points.setText(std::to_string(most));
            total = most * m_rate;
        }
        if (total == 0 || m_points.text().empty()) {
            return;
        }
        const std::int64_t asked = std::stoll(m_points.text());
        if (m_hooks.imPoints() - asked < 0) {
            m_messages.show("Not enough points", 1000); // message display time in ms
            return;
        }
        const std::string deduct = "Will deduct #R#B" + m_points.text() + "/#R/#B Point exchange for #R#B" + std::to_string(total) + "/#R/#B Gold ";
        const std::string fee = feeFree() ? "" : "#R#B(Pay host 1000)/#R/#B";
        m_hooks.ask(deduct + fee + "Are you sure?", [this] { send(); }, [this] { hide(); });
    }

    void PointExchangeWindow::send() {
        m_services.connection.send(protocol::pointExchangeRequest(m_points.text().empty() ? 0 : std::stoi(m_points.text())));
        hide();
    }

    void PointExchangeWindow::goldReceived(std::int32_t gold, std::int32_t gained) {
        m_hooks.setGold(gold);
        m_hooks.chatLine("Exchange for gold successfully " + std::to_string(gained));
    }

    void PointExchangeWindow::feeMessage(int kind, std::int32_t playerId, std::int32_t value) {
        switch (kind) {
            case 1: m_hooks.chatLine("Pay Procedure fee 1000 gold "); break; // 23/202 kind 1: the procedure fee is paid (chat line)
            case 2:
                m_messages.show(m_hooks.playerName(playerId) + " Cash machine is busy, can't be used.", 1000);
                break; // 23/202 kind 2: the ATM is busy (1000 = message ms)
            case 3: // 23/202 kind 3: fee received, sets the gold to value
                m_hooks.setGold(value);
                m_hooks.chatLine("Received Procedure fee 1000 gold ");
                break;
            case 4: // 23/202 kind 4: sets the ATM gold to value, fee must be moved to the ATM
                m_hooks.setAtmGold(value);
                m_hooks.chatLine("Can't take more money , so you need to transfer procedure fee 1000 to ATM");
                break;
            case 5: m_messages.show(" Hasen't been used", 1000); break; // 23/202 kind 5: the ATM was not used (1000 = message ms)
            default: break; // other kinds: ignored
        }
    }

    void PointExchangeWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const std::string rate = std::to_string(m_rate);
        // 8 = pixel width of one digit
        canvas
            .drawText(at + m_rateSpot.at(), rate, gfx::TextStyle{m_rateSpot.color(), gfx::TextEffect::Plain, std::nullopt}, static_cast<int>(rate.size()) * 8);
        if (m_points.text().empty()) {
            return;
        }
        if (!feeFree()) {
            if (const auto mark = m_services.pictures.find("Form_headling")) {
                const core::Rect& place = m_feeMarkSpot.rect();
                canvas.drawPicture(*mark, at + m_feeMarkSpot.at(), core::Rect{0, 0, place.width, place.height});
            }
        }
        const std::string gold = std::to_string(goldOfPoints());
        // 8 = pixel width of one digit
        canvas
            .drawText(at + m_goldSpot.at(), gold, gfx::TextStyle{m_goldSpot.color(), gfx::TextEffect::Plain, std::nullopt}, static_cast<int>(gold.size()) * 8);
    }

}
