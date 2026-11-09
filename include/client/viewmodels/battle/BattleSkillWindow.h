#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_BATTLE_BATTLE_SKILL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_BATTLE_BATTLE_SKILL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/battle/Battle.h"
#include "client/models/character/CharacterLook.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/pets/PetTeam.h"
#include "gfx/Canvas.h"
#include "ui/BlockList.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief Battle skill window: the commanding fighter's last three skills and its battle skills, with icons, names and SP.
    /// @details Original: BattleSkillWindow.xml, TRe_BattleSkillForm; opened by the ring's seventh button.
    class BattleSkillWindow {
        public:
            /// @brief Builds the window and wires its lists and buttons.
            /// @param services Game services.
            /// @param screen Screen the window lives on.
            /// @param battle The running battle.
            /// @param status Own player status (learnt skills).
            /// @param pets The player's pet team.
            /// @param look The player's look (role skill).
            BattleSkillWindow(
                GameServices& services,
                ui::Screen& screen,
                Battle& battle,
                const PlayerStatus& status,
                const PetTeam& pets,
                const CharacterLook& look
            );

            /// @brief Opens the window, or closes it when open.
            /// @details Original: ToggleBattleSkillForm.
            void toggle();

            /// @brief Hides the window.
            void hide() { m_window.setVisible(false); }

            /// @brief Loads the player's last three skills as the server keeps them.
            /// @details Original: LoadRecentBattleSkills (server 5/11).
            /// @param skills The three skill ids.
            void setRecent(const std::array<int, 3>& skills);

            /// @brief Records a skill the player or the fighting pet was commanded to use, newest first.
            /// @details Original: PushRecentBattleSkill.
            /// @param skillId Skill id.
            /// @param pet True when the pet used it.
            void pushRecent(int skillId, bool pet);

            /// @brief Gives the recent skills of the player or the pet.
            /// @details Original: FPlayerQuickSkills / FPetQuickSkills.
            /// @param pet True for the pet's list.
            /// @return The three recent skill ids.
            const std::array<int, 3>& recent(bool pet) const { return pet ? m_petRecent : m_playerRecent; }

            /// @brief Closes the window once the battle has no commander or a skill is chosen.
            /// @details Original: UpdateBattleSkillIcons, ChooseBattleSkill.
            void update();

        private:
            /// @brief Fills the skill list of the commanding fighter.
            /// @details Original: FillBattleSkillList.
            void fill();

            /// @brief Fills the list of recent skills.
            /// @details Original: RebuildPlayerRecentSkills / RebuildPetRecentSkills.
            void fillRecent();

            /// @brief Builds a row with the icon, "name   n sp" and the level under the icon.
            /// @details Original: AddBattleSkillListItem.
            /// @param skillId Skill id.
            /// @param level Skill level.
            /// @return The row.
            ui::BlockList::Row makeRow(int skillId, int level) const;

            /// @brief Gives a listed skill's level (the player's by key, the pet's by its place among its kind's).
            /// @param skillId Skill id.
            /// @param pet True for the pet's skills.
            /// @return The level, 0 when unknown.
            int levelOf(int skillId, bool pet) const;

            /// @brief Tells whether the commanding fighter is a pet.
            /// @return True when a pet commands.
            bool petCommands() const;

            /// @brief Greys the icons of the skills the commander cannot use now.
            /// @details Original: UpdateBattleSkillIcons.
            /// @param list The list to update.
            /// @param skillIds Skill of each row.
            void markUsable(ui::BlockList& list, const std::vector<int>& skillIds);

            /// @brief Paints the description of the hovered skill.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Window origin.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Chooses the skill of a row and closes the window.
            /// @param skillIds Skill of each row.
            /// @param index Row index.
            void pick(const std::vector<int>& skillIds, int index);

            GameServices& m_services;
            ui::Screen& m_screen;
            Battle& m_battle;
            const PlayerStatus& m_status;
            const PetTeam& m_pets;
            const CharacterLook& m_look;
            ui::Window& m_window;
            ui::BlockList& m_list;
            ui::BlockList& m_recent;
            const ui::Spot& m_textColorSpot;
            const ui::Spot& m_levelColorSpot;
            std::vector<int> m_listed;
            std::vector<int> m_recentListed;
            std::array<int, 3> m_playerRecent{};
            std::array<int, 3> m_petRecent{};
            int m_playerTop = 0;
            int m_petTop = 0;
    };

}

#endif
