#include "client/rules/CombatStats.h"

#include <algorithm>
#include <cmath>

namespace client {

    namespace {

        constexpr int kFactorsPerValue = 4;
        constexpr int kStrongElements[5] = {3, 1, 3, 1, 4};

        /// @brief Rounds half up.
        /// @details Original: RoundHalfUp.
        /// @param value Value to round.
        /// @return The rounded integer.
        int roundHalfUp(double value) {
            double whole = 0;
            return std::modf(value, &whole) >= 0.5 ? static_cast<int>(std::ceil(value)) : static_cast<int>(std::floor(value));
        }

    }

    int combatValue(
        const formats::FormulaData& formula,
        int which,
        int totalLevel,
        int stat,
        int bonus,
        int element
    ) {
        const std::size_t first = static_cast<std::size_t>(which * kFactorsPerValue);
        const auto& factors = formula.combatFactors;
        const int levelTerm = element == kStrongElements[which] ? 1 : 0;
        // per value 4 factors: stat factor, level factor (+1), bonus factor (+2), element level term factor (+3)
        const int value =
            roundHalfUp(totalLevel * factors[first + 1] + stat * factors[first] + bonus * factors[first + 2] + totalLevel * (levelTerm * factors[first + 3]));
        return std::max(value, 0);
    }

}
