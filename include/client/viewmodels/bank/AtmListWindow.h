#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_ATM_LIST_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_ATM_LIST_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/social/FriendList.h"
#include "client/models/guild/Guild.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/CheckBox.h"
#include "ui/ListBox.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief The ATM's list of players: friends and guild members, each kind ticked on or off; a double click gives an online player's name to the transfer form.
    /// @details Original: AtmListWindow.xml, TATMSubForm.
    class AtmListWindow {
        public:
            /// @brief Callbacks into the game: player lookup, own id and the name hand-over.
            struct Hooks {
                std::function<std::optional<std::int32_t>(const std::string&)> findPlayer;
                std::function<std::int32_t()> ownId;
                std::function<void(const std::string&)> pick;
            };

            /// @brief Loads the list and wires its events.
            /// @param services Game services.
            /// @param screen Screen the list is loaded into.
            /// @param friends The friend list.
            /// @param guild The guild.
            /// @param messages Short messages to the player.
            /// @param hooks Callbacks into the game.
            AtmListWindow(
                GameServices& services,
                ui::Screen& screen,
                const FriendList& friends,
                const Guild& guild,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the list is showing.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Fills the names and shows the list.
            /// @details Original: ShowCtrl with FillTransferPlayerList.
            void open();

            /// @brief Hides the list.
            void close();

        private:
            /// @brief Fills the names from the friends and guild members, as ticked.
            /// @details Original: FillTransferPlayerList and ToggleTransferFilter.
            void fillNames();

            /// @brief Gives the name of an online player to the transfer form and closes the list.
            /// @details Original: PickListPlayer.
            /// @param row Row picked.
            void pick(int row);

            ui::Screen& m_screen;
            const FriendList& m_friends;
            const Guild& m_guild;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::ListBox& m_names;
            ui::CheckBox& m_friendsTick;
            ui::CheckBox& m_guildTick;
    };

}

#endif
