#include "client/rules/Skills.h"

#include <algorithm>
#include <array>

namespace client {

    namespace {

        /// @brief The role of a body type and head.
        /// @details Original: GetRoleIndex.
        /// @param bodyType The body type, 1..4.
        /// @param head The head, 0..7.
        /// @return The role 1..15, 0 for none.
        int roleIndex(int bodyType, int head) {
            // roles by body type (row) and head (column); 0 for no role; row 0 unused, rows 1..4 are body types 1..4
            static const std::array<std::array<int, 8>, 5> kRoles = {{
                {},
                {3, 0, 0, 0, 0, 0, 0, 0},
                {1, 2, 0, 0, 0, 0, 0, 0},
                {6, 7, 12, 14, 0, 0, 0, 0},
                {10, 5, 4, 13, 8, 9, 11, 15},
            }};
            if (bodyType < 1 || bodyType > 4 || head < 0 || head > 7) { // body types 1..4, heads 0..7
                return 0;
            }
            return kRoles[static_cast<std::size_t>(bodyType)][static_cast<std::size_t>(head)];
        }

        // the role's own skill, indexed by role 0..15 (role 0 = none); in order:
        // 15041 Love Wish, 12036 Leap, 11075 Summon Dogs Groups, 15040 Palm, 12053 Gallop, 15038 Overarm Stumble, 11076 Combo x3 Attack,
        // 12051 Note, 12049 Fire Dance, 11078 Bomber Attack, 11077 Cure 2 Players, 15039 Wine Flame, 15060 Throw Dish, 11182 Ghost Hammer,
        // 11183 Deacon Attack
        constexpr std::array<int, 16> kRoleSkills =
            {0, 15041, 12036, 11075, 15040, 12053, 15038, 11076, 12051, 12049, 11078, 11077, 15039, 15060, 11182, 11183};

        // skills that always sit on the aid tab: 17019 Zavrl's Wing, 15222 Collapse, 15225 Curdle, 25067 Fervidity, 15166 Fast, 25147 Deadly Hit
        constexpr std::array<int, 6> kAidAnyway = {17019, 15222, 15225, 25067, 15166, 25147};

    }

    int roleSkillId(int bodyType, int head) { return kRoleSkills[static_cast<std::size_t>(roleIndex(bodyType, head))]; }

    bool isRoleSkill(int skillId) { return skillId != 0 && std::find(kRoleSkills.begin() + 1, kRoleSkills.end(), skillId) != kRoleSkills.end(); }

    bool skillOnTab(int tab, int skillId, int skillType) {
        const bool aidAnyway = std::find(kAidAnyway.begin(), kAidAnyway.end(), skillId) != kAidAnyway.end();
        switch (tab) {
            case 1: // tab 1: physical skills (skill type 1)
            case 2: // tab 2: magical skills (skill type 2)
                return skillType == tab && !aidAnyway;
            case 3: // tab 3: aid skills (skill types 3-9, 14-16, 18-21)
                return (skillType >= 3 && skillType <= 9) || (skillType >= 14 && skillType <= 16) || (skillType >= 18 && skillType <= 21) || aidAnyway;
            case 4: // tab 4: daily skills (skill type 13)
                return skillType == 13;
            default: // any other tab: none
                return false;
        }
    }

    bool petSkillOnTab(int tab, int skillType) {
        switch (tab) {
            case 1: // tab 1: physical skills (skill type 1)
            case 2: // tab 2: magical skills (skill type 2)
                return skillType == tab;
            case 3: // tab 3: aid skills (skill types 3-9, 14-16, 18-19)
                return (skillType >= 3 && skillType <= 9) || (skillType >= 14 && skillType <= 16) || (skillType >= 18 && skillType <= 19);
            case 4: // tab 4: daily skills (skill type 13)
                return skillType == 13;
            default: // any other tab: none
                return false;
        }
    }

}
