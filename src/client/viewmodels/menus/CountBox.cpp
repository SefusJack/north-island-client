#include "client/viewmodels/menus/CountBox.h"

#include <string>

namespace client {

    namespace {

        /// @brief Loads CountBox.xml into the screen and returns its window.
        /// @param services Shared game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The "CountBox" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("CountBox.xml"), services.loadContext());
            return screen.get<ui::Window>("CountBox");
        }

        constexpr std::size_t kLongestNumber = 9;

    }

    CountBox::CountBox(
        GameServices& services,
        ui::Screen& screen
    )
        : m_screen(screen),
          m_window(loadWindow(services, screen)),
          m_count(screen.get<ui::TextBox>("CountText")) {
        screen.on("onCountOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onCountCancel", [this](const ui::EventArgs&) { close(); });
        screen.on("onCountMore", [this](const ui::EventArgs&) { step(1); });
        screen.on("onCountLess", [this](const ui::EventArgs&) { step(-1); });
    }

    void CountBox::ask(int count, Confirm confirm) {
        m_count.setText(std::to_string(count));
        ask(std::move(confirm));
    }

    void CountBox::ask(Confirm confirm) {
        m_confirm = std::move(confirm);
        if (m_count.text().empty()) {
            m_count.setText("1");
        }
        m_window.setVisible(true);
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_window.id());
        }
    }

    void CountBox::step(int by) {
        const std::string& text = m_count.text();
        const int count = text.empty() || text.size() > kLongestNumber ? 0 : std::stoi(text);
        if (count + by >= 0) {
            m_count.setText(std::to_string(count + by));
        }
    }

    void CountBox::confirm() {
        const std::string& text = m_count.text();
        if (text.empty() || text.size() > kLongestNumber) {
            m_count.setText("0");
            close();
            return;
        }
        if (!m_confirm || m_confirm(std::stoi(text))) {
            close();
        }
    }

}
