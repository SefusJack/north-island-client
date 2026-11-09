#ifndef NORTH_ISLAND_CLIENT_MODELS_GUILD_EMBLEM_CELLS_H
#define NORTH_ISLAND_CLIENT_MODELS_GUILD_EMBLEM_CELLS_H

#include "core/Point.h"

#include <vector>

namespace client {

    /// @brief The cells of a line between two cells; a start left of the grid gives nothing.
    /// @details Original: CalcLineCells.
    /// @param from Start cell.
    /// @param to End cell.
    /// @return The cells on the line.
    std::vector<core::Point> lineCells(core::Point from, core::Point to);

    /// @brief The outline cells of the rectangle with the two cells as corners; a start left of the grid gives nothing.
    /// @details Original: CalcRectCells.
    /// @param from One corner.
    /// @param to The opposite corner.
    /// @return The cells of the outline.
    std::vector<core::Point> rectangleCells(core::Point from, core::Point to);

    /// @brief The outline cells of the ellipse in the rectangle of the two cells.
    /// @details Original: CalcEllipseCells.
    /// @param from One corner of the bounding rectangle.
    /// @param to The opposite corner.
    /// @return The cells of the outline.
    std::vector<core::Point> ellipseCells(core::Point from, core::Point to);

}

#endif
