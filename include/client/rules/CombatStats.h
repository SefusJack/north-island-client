#ifndef NORTH_ISLAND_CLIENT_RULES_COMBAT_STATS_H
#define NORTH_ISLAND_CLIENT_RULES_COMBAT_STATS_H

#include "formats/FormulaData.h"

namespace client {

    /// @brief Computes one combat value of a pet from the growth constants.
    /// @details Original: GetHumanAttr 41..45 for a character without a job that rides nothing. The element adds a level term to the value it is strong in (fire ATK and MATK, earth DEF and MDEF, wind SPD).
    /// @param formula Formula data with the combat factors.
    /// @param which 0 ATK (from STR), 1 DEF (CON), 2 MATK (INT), 3 MDEF (WIS), 4 SPD (AGI).
    /// @param totalLevel The total level.
    /// @param stat The base stat the value grows from.
    /// @param bonus Bonus points added.
    /// @param element The pet's element.
    /// @return The value, never below 0.
    int combatValue(
        const formats::FormulaData& formula,
        int which,
        int totalLevel,
        int stat,
        int bonus,
        int element
    );

}

#endif
