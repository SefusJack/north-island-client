#include "client/viewmodels/menus/NpcTeamMenu.h"

namespace client {

    namespace {

        /// @brief Loads NpcTeamMenu.xml into the screen and returns its window.
        /// @param services Shared game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The "NpcTeam" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("NpcTeamMenu.xml"), services.loadContext());
            return screen.get<ui::Window>("NpcTeam");
        }

    }

    NpcTeamMenu::NpcTeamMenu(
        GameServices& services,
        ui::Screen& screen
    )
        : m_screen(screen),
          m_window(loadWindow(services, screen)),
          m_text(screen.get<ui::Label>("NpcTeamText")) {
        screen.get<ui::Label>("NpcTeamJoinCaption").setText("Join");
        screen.get<ui::Label>("NpcTeamCloseCaption").setText("No");
        screen.on("onNpcTeamJoin", [this](const ui::EventArgs&) { close(); });
        screen.on("onNpcTeamClose", [this](const ui::EventArgs&) { close(); });
    }

    void NpcTeamMenu::ask(const std::string& npcName) {
        constexpr std::size_t kFits = 20; // the question is cut to 20 characters (what fits in the label)
        m_text.setText(("Do you want" + npcName + "to join the team?").substr(0, kFits));
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void NpcTeamMenu::close() { m_window.setVisible(false); }

}
