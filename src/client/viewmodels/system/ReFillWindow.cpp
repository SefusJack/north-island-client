#include "client/viewmodels/system/ReFillWindow.h"

#include "protocol/ReFillCardRequest.h"
#include "ui/TextBox.h"

#include <utility>

namespace client {

    namespace {

        constexpr int kShortMs = 2000; // message time for input hints, ms
        constexpr int kRefusedMs = 3000; // message time for the "cannot close" refusal, ms

        /// @brief Loads ReFillWindow.xml into the screen.
        /// @param services Shared game services (layout path and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The ReFill window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ReFillWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ReFill");
        }

    }

    ReFillWindow::ReFillWindow(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        Ask ask
    )
        : m_services(services),
          m_screen(screen),
          m_messages(messages),
          m_ask(std::move(ask)),
          m_window(loadWindow(services, screen)) {
        screen.on("onReFillOk", [this](const ui::EventArgs&) { this->ask(); });
        screen.on("onReFillCancel", [this](const ui::EventArgs&) {
            clear();
            hide();
        });
        screen.on("onReFillClose", [this](const ui::EventArgs&) { hide(); });
        screen.on("onReFillNext", [this](const ui::EventArgs& args) { next(args.source); });
    }

    void ReFillWindow::open() {
        m_mayClose = false;
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void ReFillWindow::close() {
        m_mayClose = true;
        clear();
        hide();
    }

    void ReFillWindow::hide() {
        if (!m_mayClose) {
            m_messages.show("Not enough points or the subscription has expired.", kRefusedMs);
            return;
        }
        m_window.setVisible(false);
    }

    void ReFillWindow::clear() {
        m_screen.get<ui::TextBox>("ReFillCard").clear();
        m_screen.get<ui::TextBox>("ReFillPassword").clear();
    }

    void ReFillWindow::ask() {
        m_ask("Please reserve enough space for gifts", [this] { submit(); });
    }

    void ReFillWindow::submit() {
        const ui::TextBox& card = m_screen.get<ui::TextBox>("ReFillCard");
        const ui::TextBox& password = m_screen.get<ui::TextBox>("ReFillPassword");
        if (card.text().empty() && password.text().empty()) {
            return;
        }
        if (card.text().empty()) {
            m_messages.show("Please enter the card number.", kShortMs);
            m_screen.focus("ReFillCard");
        } else if (password.text().empty()) {
            m_messages.show("Please enter the password.", kShortMs);
            m_screen.focus("ReFillPassword");
        } else {
            m_services.connection.send(protocol::ReFillCardRequest{card.text(), password.text()}.encode());
            m_mayClose = true;
            clear();
            hide();
        }
    }

    void ReFillWindow::next(const std::string& field) {
        if (field == "ReFillCard") {
            m_screen.focus("ReFillPassword");
            return;
        }
        ask();
        m_screen.focus("ReFillCard");
    }

}
