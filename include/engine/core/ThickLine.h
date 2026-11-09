#ifndef NORTH_ISLAND_CORE_THICK_LINE_H
#define NORTH_ISLAND_CORE_THICK_LINE_H

#include "core/Point.h"
#include "core/Rect.h"

#include <vector>

namespace core {

    /// @brief Gives the pixels a wide pen covers going from one point to another, one rectangle a row from the top down.
    /// @param from Start point.
    /// @param to End point.
    /// @param width Pen width in pixels.
    /// @return The row rectangles (a GDI pen of that width: round ends, every pixel whose centre is within half the width of the segment); empty for width < 1.
    std::vector<Rect> thickLineRows(Point from, Point to, int width);

}

#endif
