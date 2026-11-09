#include "client/viewmodels/login/IdAndPwWindow.h"

#include "protocol/IdAndPwRequests.h"
#include "ui/TextBox.h"

namespace client {

    namespace {

        constexpr int kShortMs = 1500; // how long a player message stays up, ms
        constexpr std::size_t kShortestPassword = 6; // shortest allowed password, characters
        constexpr std::size_t kLongestPassword = 14; // longest allowed password, characters (the message text says 10)

        /// @brief Loads IdAndPwWindow.xml and finds its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The IdAndPw window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("IdAndPwWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("IdAndPw");
        }

    }

    IdAndPwWindow::IdAndPwWindow(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages
    )
        : m_services(services),
          m_screen(screen),
          m_messages(messages),
          m_window(loadWindow(services, screen)) {
        screen.on("onIdAndPwOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onIdAndPwCancel", [this](const ui::EventArgs&) { hide(); });
        screen.on("onIdAndPwNext", [this](const ui::EventArgs& args) { next(args.source); });
    }

    void IdAndPwWindow::open(int mode) {
        m_mode = mode;
        m_screen.get<ui::TextBox>("IdAndPwId").clear();
        m_screen.get<ui::TextBox>("IdAndPwPw").clear();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    bool IdAndPwWindow::validAccount(const std::string& account) {
        if (account.compare(0, 2, "WL") != 0) { // length of the "WL" prefix
            return false;
        }
        std::size_t at = 2; // skip the "WL" prefix
        if (at < account.size() && (account[at] == '+' || account[at] == '-')) {
            ++at;
        }
        if (at >= account.size()) {
            return false;
        }
        for (; at < account.size(); ++at) {
            if (account[at] < '0' || account[at] > '9') {
                return false;
            }
        }
        return true;
    }

    void IdAndPwWindow::confirm() {
        if (m_mode < 1 || m_mode > 2) { // valid modes are 1 and 2 (server 21/1, 21/2)
            return;
        }
        ui::TextBox& account = m_screen.get<ui::TextBox>("IdAndPwId");
        const ui::TextBox& password = m_screen.get<ui::TextBox>("IdAndPwPw");
        if (account.text().compare(0, 2, "WL") != 0) { // length of the "WL" prefix
            m_messages.show("wrong account", kShortMs);
            account.clear();
        } else if (password.text().empty()) {
            m_messages.show("Please enter the password", kShortMs);
            account.clear();
        } else {
            m_services.connection.send(protocol::idAndPwRequest(m_mode, account.text(), password.text()));
            hide();
        }
    }

    void IdAndPwWindow::next(const std::string& field) {
        if (m_mode < 1 || m_mode > 2) { // valid modes are 1 and 2 (server 21/1, 21/2)
            return;
        }
        ui::TextBox& account = m_screen.get<ui::TextBox>("IdAndPwId");
        ui::TextBox& password = m_screen.get<ui::TextBox>("IdAndPwPw");
        if (field == "IdAndPwId") {
            if (!validAccount(account.text())) {
                m_messages.show("wrong account", kShortMs);
                account.clear();
                return;
            }
            m_screen.focus("IdAndPwPw");
            return;
        }
        if (password.text().empty()) {
            m_messages.show("Please enter the password", kShortMs);
            password.clear();
            return;
        }
        if (password.text().size() < kShortestPassword) {
            m_messages.show("The password is too short.", kShortMs);
            password.clear();
            return;
        }
        if (password.text().size() > kLongestPassword) {
            m_messages.show("The password is more than 10 letters.", kShortMs);
            password.clear();
            return;
        }
        confirm();
    }

}
