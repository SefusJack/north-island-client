#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MENUS_NPC_TEAM_MENU_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MENUS_NPC_TEAM_MENU_H

#include "client/services/GameServices.h"
#include "ui/Label.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>
#include <string>

namespace client {

    /// @brief Question whether an NPC may join the team.
    /// @details Original: NpcTeamMenu.xml, TCY_FNpcTeamMenu in its Join mode, opened by AskNpcJoinTeam. Join closes it
    /// (the game's JoinTeamClick sends nothing: it has no message 15/1) and so does Close.
    class NpcTeamMenu {
        public:
            /// @brief Loads the menu layout and wires its buttons.
            /// @param services Shared game services.
            /// @param screen Screen the menu is loaded into.
            NpcTeamMenu(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Tells whether the menu is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the question for the NPC with this name.
            /// @param npcName Name of the NPC.
            void ask(const std::string& npcName);

        private:
            /// @brief Hides the menu.
            void close();

            ui::Screen& m_screen;
            ui::Window& m_window;
            ui::Label& m_text;
    };

}

#endif
