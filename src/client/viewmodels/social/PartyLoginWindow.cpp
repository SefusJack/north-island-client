#include "client/viewmodels/social/PartyLoginWindow.h"

#include "ui/CheckBox.h"
#include "ui/Label.h"
#include "ui/TextBox.h"

namespace client {

    namespace {

        /// @brief Loads PartyLoginWindow.xml into the screen and returns its window.
        /// @param services Shared game services (layout and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The PartyLogin window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PartyLoginWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("PartyLogin");
        }

    }

    PartyLoginWindow::PartyLoginWindow(
        GameServices& services,
        ui::Screen& screen
    )
        : m_services(services),
          m_screen(screen),
          m_window(loadWindow(services, screen)) {
        screen.on("onPartyLoginOk", [this](const ui::EventArgs&) { submit(); });
        screen.on("onPartyLoginCancel", [this](const ui::EventArgs&) { close(); });
        screen.on("onPartyLoginNext", [this](const ui::EventArgs& args) { next(args.source); });
    }

    void PartyLoginWindow::open() {
        m_screen.get<ui::TextBox>("PartyLoginAccount").setText("");
        m_screen.get<ui::TextBox>("PartyLoginPassword").setText("");
        m_screen.get<ui::CheckBox>("PartyLoginSecond").setChecked(false);
        m_screen.get<ui::Label>("PartyLoginError").setText("");
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_screen.focus("PartyLoginAccount");
    }

    void PartyLoginWindow::close() {
        m_screen.get<ui::TextBox>("PartyLoginPassword").setText("");
        m_window.setVisible(false);
    }

    void PartyLoginWindow::next(const std::string& field) {
        if (field == "PartyLoginAccount") {
            m_screen.focus("PartyLoginPassword");
            return;
        }
        submit();
    }

    void PartyLoginWindow::submit() {
        if (!m_services.partyLogIn) {
            close();
            return;
        }
        const std::string error = m_services.partyLogIn(
            m_screen.get<ui::TextBox>("PartyLoginAccount").text(),
            m_screen.get<ui::TextBox>("PartyLoginPassword").text(),
            m_screen.get<ui::CheckBox>("PartyLoginSecond").isChecked() ? 2 : 1 // character number to log in: 2 for the second character, else 1
        );
        if (!error.empty()) {
            m_screen.get<ui::Label>("PartyLoginError").setText(error);
            return;
        }
        close();
    }

}
