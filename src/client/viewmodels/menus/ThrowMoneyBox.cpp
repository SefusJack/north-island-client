#include "client/viewmodels/menus/ThrowMoneyBox.h"

#include "protocol/ItemRequests.h"

#include <string>

namespace client {

    namespace {

        constexpr int kLeast = 10; // least gold that can be thrown
        constexpr int kNoTradeMap = 10701; // map 10701: the jail (name not in the tables), no throwing money there

        /// @brief Loads ThrowMoneyBox.xml and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "ThrowMoney" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ThrowMoneyBox.xml"), services.loadContext());
            return screen.get<ui::Window>("ThrowMoney");
        }

    }

    ThrowMoneyBox::ThrowMoneyBox(
        GameServices& services,
        ui::Screen& screen,
        const PlayerStatus& status,
        PlayerMessages& messages,
        std::function<int()> mapId
    )
        : m_services(services),
          m_status(status),
          m_messages(messages),
          m_mapId(std::move(mapId)),
          m_window(loadWindow(services, screen)),
          m_amount(screen.get<ui::TextBox>("ThrowMoneyAmount")) {
        screen.on("onThrowMoneyMore", [this](const ui::EventArgs&) { m_amount.setText(std::to_string(amount() + 1)); });
        screen.on("onThrowMoneyLess", [this](const ui::EventArgs&) {
            const int value = amount();
            if (value > kLeast) {
                m_amount.setText(std::to_string(value - 1));
            }
        });
        screen.on("onThrowMoneyOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onThrowMoneyCancel", [this](const ui::EventArgs&) { close(); });
    }

    void ThrowMoneyBox::toggle() {
        if (isOpen()) {
            close();
            return;
        }
        m_amount.setText(std::to_string(kLeast));
        m_window.setVisible(true);
    }

    int ThrowMoneyBox::amount() {
        if (m_amount.text().empty()) {
            m_amount.setText("0");
        }
        return std::stoi(m_amount.text());
    }

    void ThrowMoneyBox::confirm() {
        // more than 9 digits would overflow an int
        if (m_amount.text().empty() || m_amount.text().size() > 9) {
            m_amount.setText("0");
            return;
        }
        const int value = std::stoi(m_amount.text());
        if (value > m_status.gold) {
            m_messages.show("Not enough money", 1200); // message duration in ms
            return;
        }
        if (m_mapId() == kNoTradeMap) {
            m_messages.show("Unable to use the function here.", 1200); // message duration in ms
            return;
        }
        if (value >= kLeast) {
            m_services.connection.send(protocol::throwMoneyRequest(value));
        }
    }

}
