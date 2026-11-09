#ifndef NORTH_ISLAND_CLIENT_RULES_POTENTIAL_RULES_H
#define NORTH_ISLAND_CLIENT_RULES_POTENTIAL_RULES_H

#include "client/models/character/PlayerStatus.h"
#include "formats/SkillTable.h"

#include <array>
#include <vector>

namespace client {

    constexpr int kMostPotential = 12; // highest potential level (0..12)
    // stat points each potential level requires, indexed by potential level 0..12
    constexpr std::array<int, kMostPotential + 1> kPetStatRequirements = {0, 1, 2, 3, 4, 6, 9, 13, 18, 24, 31, 39, 48};

    /// @brief Lists the learned skills a character with these abilities could no longer use, with what each grants that is learned too.
    /// @details Original: CollectLostSkills. Potential (HL_Menu.pas TPotentialForm).
    /// @param skills The skill table.
    /// @param status The player's status (learned skills).
    /// @param element The element's skill tree (1..4).
    /// @param stats The abilities (STR, CON, INT, WIS, AGI).
    /// @return Skill ids that would be lost.
    std::vector<int> lostSkills(
        const formats::SkillTable& skills,
        const PlayerStatus& status,
        int element,
        const std::array<int, 5>& stats // five abilities: STR, CON, INT, WIS, AGI
    );

}

#endif
