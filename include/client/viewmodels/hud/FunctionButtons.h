#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_FUNCTION_BUTTONS_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_FUNCTION_BUTTONS_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"

#include <functional>
#include <vector>

namespace client {

    /// @brief The button row at the top right of the world: PK, combat, team, friends, money, event schedule, online draw, item mall.
    /// @details Original: FunctionButtons.xml, RE_FuncBtnForm; the money button's hint shows the gold; the team button asks to join a team or, for a leader, invites (SetTeamButtonMode).
    class FunctionButtons {
        public:
            /// @brief Loads the button row.
            /// @param services Game services (layouts).
            /// @param screen Screen the row is loaded into.
            /// @param status Player status, for the gold shown in the money hint.
            FunctionButtons(
                GameServices& services,
                ui::Screen& screen,
                const PlayerStatus& status
            );

            /// @brief Shows the row.
            void show() { m_panel.setVisible(true); }

            /// @brief Hides the row (it steps aside during a battle).
            /// @details Original: SetMainStatusBattleMode.
            void hide() { m_panel.setVisible(false); }

            /// @brief Updates the money button's hint with the current gold.
            void refresh();

            /// @brief Sets the team button's mode.
            /// @param mode 1 join, 2 invite.
            /// @details Original: SetTeamButtonMode.
            void setTeamMode(int mode);

            /// @brief Reads the team button's mode.
            /// @return 1 join, 2 invite.
            int teamMode() const { return m_teamMode; }

            /// @brief Starts or stops the blinking of the Event Schedule button.
            /// @param blinking True to blink.
            /// @details Original: StartButtonAnim / StopButtonAnim of the Event Schedule button.
            void setScheduleBlinking(bool blinking) { m_schedule.setAnimating(blinking); }

            /// @brief Shows or hides the guild invite button; the background and the first six buttons move left to make room.
            /// @param shown True when the member may invite.
            /// @details Original: LayoutFuncButtons.
            void setGuildInvite(bool shown);

        private:
            const PlayerStatus& m_status;
            ui::Element& m_panel;
            ui::Element& m_money;
            ui::Element& m_team;
            ui::Element& m_guildInvite;
            ui::ImageButton& m_schedule;
            std::vector<std::reference_wrapper<ui::Element>> m_shifting;
            bool m_guildInviteShown = false;
            int m_teamMode = 1;
    };

}

#endif
