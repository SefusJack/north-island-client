#include "client/viewmodels/bank/AtmListWindow.h"

#include <utility>

namespace client {

    namespace {

        /// @brief Loads the list's layout and gives its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "AtmList" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("AtmListWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("AtmList");
        }

    }

    AtmListWindow::AtmListWindow(
        GameServices& services,
        ui::Screen& screen,
        const FriendList& friends,
        const Guild& guild,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_screen(screen),
          m_friends(friends),
          m_guild(guild),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_names(screen.get<ui::ListBox>("AtmListNames")),
          m_friendsTick(screen.get<ui::CheckBox>("AtmListFriends")),
          m_guildTick(screen.get<ui::CheckBox>("AtmListGuild")) {
        screen.on("onAtmListClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onAtmListUp", [this](const ui::EventArgs&) { m_names.scrollUp(); });
        screen.on("onAtmListDown", [this](const ui::EventArgs&) { m_names.scrollDown(); });
        screen.on("onAtmListFilter", [this](const ui::EventArgs&) { fillNames(); });
        screen.on("onAtmListPick", [this](const ui::EventArgs& args) { pick(args.index); });
    }

    void AtmListWindow::open() {
        fillNames();
        m_window.setVisible(true);
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_window.id());
        }
    }

    void AtmListWindow::close() { m_window.setVisible(false); }

    void AtmListWindow::fillNames() {
        m_names.clear();
        const auto listed = [this](const std::string& name) {
            for (int row = 0; row < m_names.itemCount(); ++row) {
                if (!m_names.itemText(row).empty() && m_names.itemText(row) == name) {
                    return true;
                }
            }
            return false;
        };
        if (m_friendsTick.isChecked()) {
            for (int index = 1; index <= m_friends.count(); ++index) { // friend list is numbered from 1
                const std::string& name = m_friends.at(index).record.name;
                if (!listed(name)) {
                    m_names.addItem(name);
                }
            }
        }
        if (m_guildTick.isChecked()) {
            for (int index = 0; index < m_guild.count(); ++index) {
                const protocol::GuildMemberRecord& member = m_guild.at(index).record;
                if (!listed(member.name) && member.id != m_hooks.ownId() && member.id != 0) { // member id 0 is never listed
                    m_names.addItem(member.name);
                }
            }
        }
    }

    void AtmListWindow::pick(int row) {
        if (row < 0 || row >= m_names.itemCount()) {
            return;
        }
        const std::string& name = m_names.itemText(row);
        if (!m_hooks.findPlayer(name)) {
            m_messages.show("Doesn't exist or off line", 1500); // message time in ms
            return;
        }
        m_hooks.pick(name);
        close();
    }

}
