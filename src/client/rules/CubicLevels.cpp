#include "client/rules/CubicLevels.h"

#include <cmath>

namespace client {

    std::uint32_t cubicLevelExp(int level) {
        return static_cast<std::uint32_t>(std::llround(std::pow(static_cast<double>(level), 3.0))) + 20;
    } // level cubed plus 20

    std::uint32_t totalCubicLevelExp(int level) {
        std::uint32_t total = 0;
        for (int step = 1; step <= level; ++step) {
            total += cubicLevelExp(step);
        }
        return total;
    }

    int cubicLevelFromExp(std::uint32_t experience, int level) {
        do {
            ++level;
            if (totalCubicLevelExp(level) > experience) {
                break;
            }
        } while (level <= 200); // highest level (200)
        return level - 1;
    }

}
