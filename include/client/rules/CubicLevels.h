#ifndef NORTH_ISLAND_CLIENT_RULES_CUBIC_LEVELS_H
#define NORTH_ISLAND_CLIENT_RULES_CUBIC_LEVELS_H

#include <cstdint>

namespace client {

    /// @brief Experience a level of the guild's jobs asks for: its cube and 20 more.
    /// @details Original: CubicLevelExp (TCalculator).
    /// @param level The level.
    /// @return The experience of that level.
    std::uint32_t cubicLevelExp(int level);

    /// @brief Experience that reaches a level (the sum of the levels from 1).
    /// @details Original: TotalCubicLevelExp.
    /// @param level The level.
    /// @return The total experience.
    std::uint32_t totalCubicLevelExp(int level);

    /// @brief Gives the level an experience reaches, counted up from a level already reached (200 at most).
    /// @details Original: CubicLevelFromExp.
    /// @param experience Total experience.
    /// @param level A level already reached.
    /// @return The level.
    int cubicLevelFromExp(std::uint32_t experience, int level);

}

#endif
