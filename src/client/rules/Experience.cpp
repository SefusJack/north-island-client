#include "client/rules/Experience.h"

#include <algorithm>
#include <cmath>

namespace client {

    std::int64_t levelUpExp(const formats::FormulaData& formula, int level, int reborn) {
        double power = formula.expPower;
        double addend = formula.expAdd;
        if (reborn != 0) {
            // reborn curve: exponent 3.3; addend 50 up to level 150, then (level - 150) ^ 4.9
            power = 3.3;
            addend = level <= 150 ? 50.0 : std::pow(level - 150, 4.9);
        }
        return static_cast<std::int64_t>(std::nearbyint(std::pow(level, power) + addend));
    }

    std::int64_t totalLevelExp(const formats::FormulaData& formula, int level, int reborn) {
        std::int64_t total = 0;
        for (int i = 1; i <= level; ++i) {
            total += levelUpExp(formula, i, reborn);
        }
        return total;
    }

    double levelProgress(const formats::FormulaData& formula, int level, std::int64_t exp, int reborn) {
        const std::int64_t reached = totalLevelExp(formula, level, reborn);
        const std::int64_t needed = levelUpExp(formula, level + 1, reborn);
        if (needed <= 0) {
            return 0;
        }
        return std::min(1.0, static_cast<double>(std::max(exp, reached) - reached) / static_cast<double>(needed));
    }

    double levelProgressSigned(const formats::FormulaData& formula, int level, std::int64_t exp, int reborn) {
        const std::int64_t reached = totalLevelExp(formula, level, reborn);
        const std::int64_t needed = levelUpExp(formula, level + 1, reborn);
        if (needed <= 0) {
            return 0;
        }
        return std::min(1.0, static_cast<double>(exp - reached) / static_cast<double>(needed));
    }

    std::int64_t skillLevelExp(const formats::FormulaData& formula, int level) {
        if (level == 1) {
            return 0;
        }
        return static_cast<std::int64_t>(std::nearbyint(std::pow(level, formula.expPower) + formula.expAdd));
    }

    std::int64_t totalSkillLevelExp(const formats::FormulaData& formula, int level) {
        std::int64_t total = 0;
        for (int i = 1; i <= level; ++i) {
            total += skillLevelExp(formula, i);
        }
        return total;
    }

    int levelFromExp(const formats::FormulaData& formula, std::int64_t exp, int reborn) {
        const int maxLevel = reborn > 0 ? 190 : 199; // highest level: 190 after rebirth, 199 before
        int level = 0;
        while (level < maxLevel && totalLevelExp(formula, level + 1, reborn) <= exp) {
            ++level;
        }
        return level;
    }

}
