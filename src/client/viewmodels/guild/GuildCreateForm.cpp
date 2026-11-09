#include "client/viewmodels/guild/GuildCreateForm.h"

#include "client/rules/TextFilter.h"
#include "protocol/GuildRequests.h"

#include <algorithm>

namespace client {

    namespace {

        /// @brief Loads GuildCreateForm.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "GuildCreate" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("GuildCreateForm.xml"), services.loadContext());
            return screen.get<ui::Window>("GuildCreate");
        }

    }

    GuildCreateForm::GuildCreateForm(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        std::function<void()> closed
    )
        : m_services(services),
          m_screen(screen),
          m_messages(messages),
          m_closed(std::move(closed)),
          m_window(loadWindow(services, screen)),
          m_name(screen.get<ui::TextBox>("GuildCreateName")) {
        screen.on("onGuildCreateOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onGuildCreateCancel", [this](const ui::EventArgs&) { close(); });
    }

    void GuildCreateForm::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_screen.focus(m_name.name());
    }

    std::string GuildCreateForm::takeName() {
        const std::string name = m_name.text();
        m_name.clear();
        return name;
    }

    void GuildCreateForm::close() {
        m_window.setVisible(false);
        m_closed();
    }

    void GuildCreateForm::confirm() {
        const std::string& name = m_name.text();
        if (!isNameClean(name, false)) {
            m_messages.show(kIllegalWords, 2000); // message time in ms
            return;
        }
        // 128: first non-ASCII byte value
        if (std::any_of(name.begin(), name.end(), [](char c) { return static_cast<unsigned char>(c) >= 128; })) {
            m_messages.show("Don't use special characters", 2000); // message time in ms
            return;
        }
        m_services.connection.send(protocol::createGuildRequest(name));
        close();
    }

}
