#ifndef NORTH_ISLAND_CORE_POINT_H
#define NORTH_ISLAND_CORE_POINT_H

namespace core {

    /// @brief A position in pixels.
    struct Point {
        int x = 0;
        int y = 0;

        /// @brief Adds two points.
        /// @param other Point to add.
        /// @return The sum.
        Point operator+(Point other) const { return {x + other.x, y + other.y}; }

        /// @brief Subtracts a point.
        /// @param other Point to subtract.
        /// @return The difference.
        Point operator-(Point other) const { return {x - other.x, y - other.y}; }

        /// @brief Compares both coordinates.
        /// @param other Point to compare with.
        /// @return True when equal.
        bool operator==(Point other) const { return x == other.x && y == other.y; }

        /// @brief Compares both coordinates.
        /// @param other Point to compare with.
        /// @return True when different.
        bool operator!=(Point other) const { return !(*this == other); }
    };

}

#endif
