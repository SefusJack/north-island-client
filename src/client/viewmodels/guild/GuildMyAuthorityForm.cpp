#include "client/viewmodels/guild/GuildMyAuthorityForm.h"

namespace client {

    namespace {

        constexpr int kRightInvite = 2;
        constexpr int kRightEmblem = 5;

        /// @brief Loads GuildMyAuthorityForm.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "GuildMyAuthorityForm" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("GuildMyAuthorityForm.xml"), services.loadContext());
            return screen.get<ui::Window>("GuildMyAuthorityForm");
        }

    }

    GuildMyAuthorityForm::GuildMyAuthorityForm(
        GameServices& services,
        ui::Screen& screen,
        const Guild& guild,
        Own own
    )
        : m_screen(screen),
          m_guild(guild),
          m_own(std::move(own)),
          m_window(loadWindow(services, screen)),
          m_name(screen.get<ui::TextBox>("GuildMyName")) {
        screen.on("onGuildMyAuthorityClose", [this](const ui::EventArgs&) { close(); });
    }

    void GuildMyAuthorityForm::toggle() {
        if (m_window.isVisible()) {
            close();
        } else {
            open();
        }
    }

    void GuildMyAuthorityForm::open() {
        m_name.setText(m_own.name());
        for (const char* name : {"GuildMyMark1", "GuildMyMark2", "GuildMyMark3", "GuildMyMark4", "GuildMyMark5", "GuildMyViceMark"}) {
            m_screen.get<ui::Element>(name).setVisible(false);
        }
        const int index = m_guild.indexOf(m_own.id());
        if (index >= 0) {
            const protocol::GuildMemberRecord& record = m_guild.at(index).record;
            m_screen.get<ui::Element>("GuildMyMark2").setVisible(record.rights[kRightInvite - 1]);
            m_screen.get<ui::Element>("GuildMyMark5").setVisible(record.rights[kRightEmblem - 1]);
            m_screen.get<ui::Element>("GuildMyViceMark").setVisible(index >= 1 && index <= m_guild.viceCount());
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

}
