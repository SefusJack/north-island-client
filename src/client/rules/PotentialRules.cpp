#include "client/rules/PotentialRules.h"

#include "client/data/JobGrantSkills.h"
#include "client/rules/SkillTree.h"

namespace client {

    namespace {

        constexpr int kGrantGroups = 3; // job-grant table: skill groups per element
        constexpr int kGrantRows = 10; // job-grant table: rows per group
        constexpr int kGrantColumns = 5; // job-grant table: skills per row

        /// @brief Whether the player has learned a skill.
        /// @param skills The skill table.
        /// @param status The player's status.
        /// @param skillId The skill id.
        /// @return True when learned at level 1 or more.
        bool learned(const formats::SkillTable& skills, const PlayerStatus& status, int skillId) {
            const auto record = skills.find(skillId);
            if (!record || record->key == 0) {
                return false;
            }
            const auto found = status.skills.find(record->key);
            return found != status.skills.end() && found->second.level != 0;
        }

    }

    std::vector<int> lostSkills(
        const formats::SkillTable& skills,
        const PlayerStatus& status,
        int element,
        const std::array<int, 5>& stats
    ) { // five abilities: STR, CON, INT, WIS, AGI
        std::vector<int> lost;
        if (element < 1 || element > 4) { // elements 1..4 have skill trees
            return lost;
        }
        for (int cell = 1; cell <= kSkillTreeCells; ++cell) {
            const SkillTreeCell& entry = skillTreeCell(element, cell);
            bool found = false;
            for (std::size_t stat = 0; stat < stats.size(); ++stat) {
                if (!learned(skills, status, entry.skillId)) {
                    break;
                }
                if (entry.required[stat] != 0 && entry.required[stat] > stats[stat]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                continue;
            }
            bool granted = false;
            for (int group = 1; group <= kGrantGroups && !granted; ++group) {
                for (int row = 1; row <= kGrantRows && !granted; ++row) {
                    if (jobGrantSkill(element, group, row, 1) != entry.skillId) {
                        continue;
                    }
                    granted = true;
                    for (int column = 1; column <= kGrantColumns; ++column) {
                        const int skillId = jobGrantSkill(element, group, row, column);
                        if (skillId != 0 && learned(skills, status, skillId)) {
                            lost.push_back(skillId);
                        }
                    }
                }
            }
        }
        return lost;
    }

}
