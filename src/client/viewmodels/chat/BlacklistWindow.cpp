#include "client/viewmodels/chat/BlacklistWindow.h"

#include "client/rules/GmIds.h"
#include "protocol/FriendRequests.h"
#include "ui/TextBox.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kAdd = 1;
        constexpr int kRemove = 2;

        /// @brief Loads BlacklistWindow.xml into the screen.
        /// @param services Game services (layout, load context).
        /// @param screen Screen to load into.
        /// @return The "Blacklist" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("BlacklistWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Blacklist");
        }

    }

    BlacklistWindow::BlacklistWindow(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        std::function<std::optional<std::int32_t>(const std::string& name)> findPlayer
    )
        : m_services(services),
          m_screen(screen),
          m_messages(messages),
          m_findPlayer(std::move(findPlayer)),
          m_window(loadWindow(services, screen)),
          m_names(screen.get<ui::ListBox>("BlacklistNames")),
          m_name(screen.get<ui::TextBox>("BlacklistName")) {
        screen.on("onBlacklistAdd", [this](const ui::EventArgs&) { add(); });
        screen.on("onBlacklistRemove", [this](const ui::EventArgs&) { remove(); });
        screen.on("onBlacklistClose", [this](const ui::EventArgs&) { m_window.setVisible(false); });
        screen.on("onBlacklistUp", [this](const ui::EventArgs&) { m_names.scrollUp(); });
        screen.on("onBlacklistDown", [this](const ui::EventArgs&) { m_names.scrollDown(); });
    }

    void BlacklistWindow::toggle() {
        m_window.setVisible(!m_window.isVisible());
        if (m_window.isVisible() && m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    bool BlacklistWindow::contains(std::int32_t playerId, const std::string& name) const {
        if (playerId == 0 || name.empty()) {
            return false;
        }
        return std::any_of(m_entries.begin(), m_entries.end(), [&](const protocol::BlacklistEntry& entry) {
            return entry.playerId == playerId && entry.name == name;
        });
    }

    void BlacklistWindow::showNames() {
        m_names.clear();
        for (const protocol::BlacklistEntry& entry : m_entries) {
            if (entry.playerId != 0 && !entry.name.empty()) {
                m_names.addItem(entry.name);
            }
        }
    }

    void BlacklistWindow::load(const std::vector<protocol::BlacklistEntry>& entries) {
        m_entries.clear();
        for (const protocol::BlacklistEntry& entry : entries) {
            if (isStaffId(entry.playerId)) {
                break;
            }
            m_entries.push_back(entry);
        }
        showNames();
    }

    void BlacklistWindow::update(int result, const std::vector<protocol::BlacklistEntry>& entries) {
        switch (result) {
            case 0: // 14/13 result 0: operation failed, take the server's list
                m_messages.show("Operation failed", 1000); // message display time in ms
                load(entries);
                break;
            case 1: // 14/13 result 1: the asked player goes on the list
                if (m_pending && !isStaffId(m_pending->playerId) && m_entries.size() < kMostEntries) {
                    m_entries.push_back(*m_pending);
                    m_services.playSound("sound/wav0155.wav");
                    m_messages.show("Add  [" + m_pending->name + "] into blacklist", 2400); // message display time in ms
                }
                showNames();
                break;
            case 2: // 14/13 result 2: the asked player goes off the list
                if (m_pending) {
                    m_entries.erase(
                        std::remove_if(
                            m_entries.begin(),
                            m_entries.end(),
                            [&](const protocol::BlacklistEntry& entry) {
                                return (entry.playerId == m_pending->playerId && entry.name == m_pending->name) || isStaffId(entry.playerId);
                            }
                        ),
                        m_entries.end()
                    );
                    m_messages.show("Remove player [" + m_pending->name + "] from blacklist", 2400); // message display time in ms
                }
                showNames();
                break;
            case 3: // 14/13 result 3: take the server's list
                load(entries);
                break;
            default: // unknown result: ignored
                break;
        }
        m_pending.reset();
    }

    void BlacklistWindow::add() {
        if (m_pending) {
            return;
        }
        const std::string name = m_name.text();
        if (name.empty()) {
            m_messages.show("Please enter player's name", 1000); // message display time in ms
            return;
        }
        if (m_names.itemCount() + 1 > static_cast<int>(kMostEntries)) {
            m_messages.show("Your blacklist is full", 1000); // message display time in ms
            return;
        }
        for (const protocol::BlacklistEntry& entry : m_entries) {
            if (entry.name == name) {
                m_messages.show("The player has been in the list", 1200); // message display time in ms
                return;
            }
        }
        const auto playerId = m_findPlayer(name);
        if (playerId && isGmId(*playerId)) {
            m_messages.show("Cannot add GM into blacklist", 1200); // message display time in ms
            return;
        }
        if (!playerId) {
            m_messages.show("Unable to find the player", 1000); // message display time in ms
            return;
        }
        m_pending = protocol::BlacklistEntry{*playerId, name};
        m_services.connection.send(protocol::blacklistRequest(kAdd, *playerId, name));
        m_name.setText("");
    }

    void BlacklistWindow::remove() {
        if (m_pending) {
            return;
        }
        const int row = m_names.selectedIndex();
        if (row < 0) {
            m_messages.show("Please select the player to remove", 1000); // message display time in ms
            return;
        }
        if (row < static_cast<int>(m_entries.size())) {
            m_pending = m_entries[static_cast<std::size_t>(row)];
            m_services.connection.send(protocol::blacklistRequest(kRemove, m_pending->playerId, m_pending->name));
        }
        m_names.setSelectedIndex(-1);
    }

}
