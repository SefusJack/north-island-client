#ifndef NORTH_ISLAND_CLIENT_RULES_SKILL_TREE_H
#define NORTH_ISLAND_CLIENT_RULES_SKILL_TREE_H

#include "core/Point.h"

#include <array>

namespace client {

    /// @brief One skill of a skill tree with the stats it needs (STR, CON, INT, WIS, AGI).
    /// @details Original: GRace_B (the skill window's Intro page).
    struct SkillTreeCell {
        int skillId = 0;
        std::array<int, 5> required{}; // STR, CON, INT, WIS, AGI
    };

    constexpr int kSkillTreeCells = 18; // skills per element tree (three columns of six)

    /// @brief A cell of an element's skill tree (18 skills in three columns of six).
    /// @param element 1 earth, 2 water, 3 fire, 4 wind.
    /// @param cell The cell, 1..18.
    /// @return The cell.
    const SkillTreeCell& skillTreeCell(int element, int cell);

    /// @brief Where a cell's icon lies in the skill window.
    /// @details Original: GPt_B.
    /// @param cell The cell, 1..18.
    /// @return The icon position.
    core::Point skillTreeCellPosition(int cell);

}

#endif
