#include "client/viewmodels/hud/FunctionButtons.h"

#include <string>

namespace client {

    FunctionButtons::FunctionButtons(
        GameServices& services,
        ui::Screen& screen,
        const PlayerStatus& status
    )
        : m_status(status),
          m_panel(screen.load(services.game.layout("FunctionButtons.xml"), services.loadContext())),
          m_money(screen.get<ui::Element>("FunctionMoney")),
          m_team(screen.get<ui::Element>("FunctionTeam")),
          m_guildInvite(screen.get<ui::Element>("FunctionGuildInvite")),
          m_schedule(screen.get<ui::ImageButton>("FunctionSchedule")) {
        m_schedule.setAnimating(false);
        for (const char* name :
             {"FunctionBackground", "FunctionPk", "FunctionJoinCombat", "FunctionWatchCombat", "FunctionTeam", "FunctionFriends", "FunctionMoney"}) {
            m_shifting.push_back(screen.get<ui::Element>(name));
        }
    }

    void FunctionButtons::setGuildInvite(bool shown) {
        if (shown == m_guildInviteShown) {
            return;
        }
        m_guildInviteShown = shown;
        constexpr int kButtonStep = 37; // px the other function buttons shift left when the guild invite button shows
        for (ui::Element& element : m_shifting) {
            const core::Rect bounds = element.bounds();
            element.setBounds({bounds.x + (shown ? -kButtonStep : kButtonStep), bounds.y, bounds.width, bounds.height});
        }
        m_guildInvite.setVisible(shown);
    }

    void FunctionButtons::setTeamMode(int mode) {
        m_teamMode = mode;
        // team mode 2 = invite (leader), otherwise join
        m_team.setHint(mode == 2 ? "Invite to the team" : "join the team");
    }

    void FunctionButtons::refresh() { m_money.setHint("Commerce Money " + std::to_string(m_status.gold)); }

}
