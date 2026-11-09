#ifndef NORTH_ISLAND_CORE_RECT_H
#define NORTH_ISLAND_CORE_RECT_H

#include "core/Point.h"

namespace core {

    /// @brief An axis-aligned rectangle: position plus size, in pixels.
    struct Rect {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;

        /// @brief Gives the left edge.
        /// @return x.
        int left() const { return x; }

        /// @brief Gives the top edge.
        /// @return y.
        int top() const { return y; }

        /// @brief Gives the right edge (exclusive).
        /// @return x + width.
        int right() const { return x + width; }

        /// @brief Gives the bottom edge (exclusive).
        /// @return y + height.
        int bottom() const { return y + height; }

        /// @brief Gives the top-left corner.
        /// @return The position.
        Point position() const { return {x, y}; }

        /// @brief Tells whether the rectangle covers no pixel.
        /// @return True when width or height is not positive.
        bool isEmpty() const { return width <= 0 || height <= 0; }

        /// @brief Tells whether a point lies inside (right and bottom edges excluded).
        /// @param p The point.
        /// @return True when inside.
        bool contains(Point p) const { return p.x >= x && p.y >= y && p.x < right() && p.y < bottom(); }

        /// @brief Moves the rectangle.
        /// @param offset Amount to move by.
        /// @return The moved rectangle.
        Rect translated(Point offset) const { return {x + offset.x, y + offset.y, width, height}; }

        /// @brief Gives the overlap with another rectangle.
        /// @param other Rectangle to overlap with.
        /// @return The overlap, an empty rectangle when they do not overlap.
        Rect intersected(const Rect& other) const;

        /// @brief Makes a rectangle from its four edges.
        /// @param left Left edge.
        /// @param top Top edge.
        /// @param right Right edge.
        /// @param bottom Bottom edge.
        /// @return The rectangle.
        static Rect fromEdges(int left, int top, int right, int bottom) { return {left, top, right - left, bottom - top}; }

        /// @brief Compares all four fields.
        /// @param other Rectangle to compare with.
        /// @return True when equal.
        bool operator==(const Rect& other) const { return x == other.x && y == other.y && width == other.width && height == other.height; }
    };

}

#endif
