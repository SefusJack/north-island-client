#ifndef NORTH_ISLAND_CORE_BETWEEN_H
#define NORTH_ISLAND_CORE_BETWEEN_H

#include "core/Point.h"
#include "core/Rect.h"

#include <cmath>
#include <cstdlib>

namespace core {

    /// @brief Gives a value part of the way from one to another, rounded to whole pixels.
    /// @param from First value (amount 0).
    /// @param to Second value (amount 1).
    /// @param amount Fraction of the way, 0..1.
    /// @return The rounded value.
    inline int between(int from, int to, double amount) { return static_cast<int>(std::lround(from + (to - from) * amount)); }

    /// @brief Gives a place part of the way, or `to` itself when the two lie further apart than snapPixels (a jump, not a move).
    /// @param from First place.
    /// @param to Second place.
    /// @param amount Fraction of the way, 0..1.
    /// @param snapPixels Largest per-axis distance that is still interpolated.
    /// @return The place.
    inline Point between(Point from, Point to, double amount, int snapPixels) {
        if (std::abs(to.x - from.x) > snapPixels || std::abs(to.y - from.y) > snapPixels) {
            return to;
        }
        return {between(from.x, to.x, amount), between(from.y, to.y, amount)};
    }

    /// @brief Gives a rectangle part of the way from one to another, each field rounded.
    /// @param from First rectangle.
    /// @param to Second rectangle.
    /// @param amount Fraction of the way, 0..1.
    /// @return The rectangle.
    inline Rect between(const Rect& from, const Rect& to, double amount) {
        return {between(from.x, to.x, amount), between(from.y, to.y, amount), between(from.width, to.width, amount), between(from.height, to.height, amount)};
    }

}

#endif
