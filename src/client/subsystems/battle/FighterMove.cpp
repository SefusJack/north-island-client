#include "client/subsystems/battle/FighterMove.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        /// @brief Rounds a double to the nearest int.
        /// @param value The value.
        /// @return The rounded value.
        int round(double value) { return static_cast<int>(std::lrint(value)); }

    }

    core::Point FighterMove::step(core::Point screen, core::Point dest, std::uint64_t nowMs) {
        const double elapsed = nowMs > m_lastMs ? static_cast<double>(nowMs - m_lastMs) : 0.0;
        m_lastMs = nowMs;
        const int step = round(elapsed * m_speed);
        const int distX = std::abs(dest.x - screen.x);
        const int distY = std::abs(dest.y - screen.y);
        if (step > distX && step > distY) {
            return dest;
        }
        // 0.001 keeps the divisors above zero
        double ratioYX = 0;
        double ratioXY = 0;
        if (distY > distX) {
            ratioYX = distY / (0.001 + distX);
        } else {
            ratioXY = distX / (0.001 + distY);
        }
        double stepX = step / (ratioYX + 0.001);
        double stepY = step / (ratioXY + 0.001);
        stepX = std::min<double>(stepX, step);
        stepY = std::min<double>(stepY, step);
        core::Point next = screen;
        if (screen.x < dest.x) {
            next.x += round(stepX);
        } else if (screen.x > dest.x) {
            next.x -= round(stepX);
        }
        if (screen.y < dest.y) {
            next.y += round(stepY);
        } else if (screen.y > dest.y) {
            next.y -= round(stepY);
        }
        return next;
    }

}
