#ifndef NORTH_ISLAND_CLIENT_RULES_VITALITY_H
#define NORTH_ISLAND_CLIENT_RULES_VITALITY_H

#include "formats/FormulaData.h"

namespace client {

    /// @brief Gives the total level (Original: CalcTotalLevel).
    /// @param level The level.
    /// @param reborn The reborn count.
    /// @return reborn * 100 + level.
    int totalLevel(int level, int reborn);

    /// @brief Gives the most HP from the growth constants (Original: TCalculator.CalcMaxHp).
    /// @param formula The growth constants.
    /// @param totalLevel The total level.
    /// @param constitution Constitution.
    /// @param bonus What equipment and the like add.
    /// @return The most HP, at least 0.
    int maxHp(const formats::FormulaData& formula, int totalLevel, int constitution, int bonus);

    /// @brief Gives the most SP from the growth constants (Original: TCalculator.CalcMaxSp).
    /// @param formula The growth constants.
    /// @param totalLevel The total level.
    /// @param wisdom Wisdom.
    /// @param bonus What equipment and the like add.
    /// @return The most SP.
    int maxSp(const formats::FormulaData& formula, int totalLevel, int wisdom, int bonus);

}

#endif
