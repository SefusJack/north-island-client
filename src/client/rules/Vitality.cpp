#include "client/rules/Vitality.h"

#include <algorithm>
#include <cmath>

namespace client {

    int totalLevel(int level, int reborn) { return reborn * 100 + level; } // each reborn adds 100 levels

    int maxHp(const formats::FormulaData& formula, int totalLevel, int constitution, int bonus) {
        const double grown = std::pow(totalLevel, formula.hpLevelPower) * (constitution * formula.hpStatScale) + totalLevel * formula.hpPerLevel +
                             constitution * formula.hpPerStat;
        return std::max(0, static_cast<int>(std::nearbyint(grown)) + formula.hpBase + bonus);
    }

    int maxSp(const formats::FormulaData& formula, int totalLevel, int wisdom, int bonus) {
        const double grown =
            std::pow(totalLevel, formula.spLevelPower) * (wisdom * formula.spStatScale) + totalLevel * formula.spPerLevel + wisdom * formula.spPerStat;
        return static_cast<int>(std::nearbyint(grown)) + formula.spBase + bonus;
    }

}
