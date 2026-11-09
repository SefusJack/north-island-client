#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_BLACKLIST_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_BLACKLIST_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/BlacklistEntry.h"
#include "ui/ListBox.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The black list window (BlacklistWindow.xml, THL_BlackNameListForm): up to 20 players whose chat is not shown.
    /// @details Original: added by name from the players online (14/5 1), removed by row (14/5 2), kept by the server (14/13).
    class BlacklistWindow {
        public:
            /// @brief Loads BlacklistWindow.xml and wires the window.
            /// @param services Game services (layout, connection, sound).
            /// @param screen Screen the window is loaded into.
            /// @param messages Short on-screen message line.
            /// @param findPlayer Finds an online player's id by name (original FindWorldPlayerByName); nullopt when none.
            BlacklistWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                std::function<std::optional<std::int32_t>(const std::string& name)> findPlayer
            );

            /// @brief Whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows or hides the window.
            /// @details Original: ToggleBlacklistForm.
            void toggle();

            /// @brief Applies the server's answer: 1 the asked one goes on, 2 off, 0 and 3 the server's list.
            /// @param result Result code of the packet.
            /// @param entries The server's list.
            /// @details Original: HandleBlacklistPacket (14/13).
            void update(int result, const std::vector<protocol::BlacklistEntry>& entries);

            /// @brief Whether a player is on the list.
            /// @param playerId Player id.
            /// @param name Player name.
            /// @return True when id and name both match an entry.
            /// @details Original: IsInBlacklist.
            bool contains(std::int32_t playerId, const std::string& name) const;

        private:
            /// @brief Asks the server to add the typed name (14/5 1) after the checks.
            /// @details Original: AddBlacklistClick.
            void add();

            /// @brief Asks the server to remove the selected row (14/5 2).
            /// @details Original: RemoveBlacklistClick.
            void remove();

            /// @brief Takes the server's list, up to the first of the game's own (staff) ids.
            /// @param entries The server's list.
            /// @details Original: RecvBlacklist.
            void load(const std::vector<protocol::BlacklistEntry>& entries);

            /// @brief Refills the name list from the entries.
            void showNames();

            static constexpr std::size_t kMostEntries = 20; // most names on the blacklist

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            std::function<std::optional<std::int32_t>(const std::string&)> m_findPlayer;
            ui::Window& m_window;
            ui::ListBox& m_names;
            ui::TextBox& m_name;
            std::vector<protocol::BlacklistEntry> m_entries;
            std::optional<protocol::BlacklistEntry> m_pending;
    };

}

#endif
