#include "client/subsystems/battle/DashArc.h"

#include <cmath>

namespace client {

    namespace {

        /// @brief Returns the rounded distance between two points.
        /// @param from Start point.
        /// @param to End point.
        /// @return The distance in pixels.
        int distance(core::Point from, core::Point to) {
            const double x = static_cast<double>(to.x - from.x);
            const double y = static_cast<double>(to.y - from.y);
            return static_cast<int>(std::lrint(std::sqrt(x * x + y * y)));
        }

    }

    int DashArc::length(core::Point start, core::Point cell) { return distance(start, cell); }

    int DashArc::lift(core::Point position, core::Point cell, int length) {
        const double remaining = static_cast<double>(distance(position, cell));
        // 0.001 keeps the divisor above zero; 360 is the peak height scale of the arc
        const int height = static_cast<int>(std::lrint((0.5 - std::abs(remaining / (length + 0.001) - 0.5)) * 360));
        return static_cast<int>(std::lrint(height - std::pow(height, 2) * 0.003)); // 0.003: how much of the squared height is taken off
    }

}
