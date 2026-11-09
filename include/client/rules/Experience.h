#ifndef NORTH_ISLAND_CLIENT_RULES_EXPERIENCE_H
#define NORTH_ISLAND_CLIENT_RULES_EXPERIENCE_H

#include "formats/FormulaData.h"

#include <cstdint>

namespace client {

    /// @brief Experience to go from level - 1 to level.
    /// @details Original: TCalculator.LevelUpExp.
    /// @param formula Formula data (exponent and addend).
    /// @param level The level reached.
    /// @param reborn Rebirth count (0 for none).
    /// @return The experience for that level.
    std::int64_t levelUpExp(const formats::FormulaData& formula, int level, int reborn);

    /// @brief Experience for levels 1..level together.
    /// @details Original: TCalculator.TotalLevelExp.
    /// @param formula Formula data.
    /// @param level The last level counted.
    /// @param reborn Rebirth count (0 for none).
    /// @return The summed experience.
    std::int64_t totalLevelExp(const formats::FormulaData& formula, int level, int reborn);

    /// @brief How far into the next level a character is.
    /// @param formula Formula data.
    /// @param level The character's level.
    /// @param exp The character's experience.
    /// @param reborn Rebirth count (0 for none).
    /// @return Progress from 0 to 1.
    double levelProgress(const formats::FormulaData& formula, int level, std::int64_t exp, int reborn);

    /// @brief The same progress without the lower bound: below the level's base experience it is negative.
    /// @details Original: the item window prints "-464%" for it (TSe_EquipUI's draw divides the difference and only caps it at 1; the bar itself draws nothing for a negative ratio).
    /// @param formula Formula data.
    /// @param level The character's level.
    /// @param exp The character's experience.
    /// @param reborn Rebirth count (0 for none).
    /// @return Progress, at most 1, possibly negative.
    double levelProgressSigned(const formats::FormulaData& formula, int level, std::int64_t exp, int reborn);

    /// @brief The level an amount of experience reaches (at most 199, 190 after rebirth).
    /// @details Original: TCalculator.LevelFromExp.
    /// @param formula Formula data.
    /// @param exp The experience.
    /// @param reborn Rebirth count (0 for none).
    /// @return The level.
    int levelFromExp(const formats::FormulaData& formula, std::int64_t exp, int reborn);

    /// @brief Experience a skill needs from level - 1 to level (level 1 needs none).
    /// @details Original: TCalculator.SkillLevelExp.
    /// @param formula Formula data.
    /// @param level The skill level reached.
    /// @return The experience for that level.
    std::int64_t skillLevelExp(const formats::FormulaData& formula, int level);

    /// @brief Experience for skill levels 1..level together.
    /// @details Original: TotalSkillLevelExp.
    /// @param formula Formula data.
    /// @param level The last level counted.
    /// @return The summed experience.
    std::int64_t totalSkillLevelExp(const formats::FormulaData& formula, int level);

}

#endif
