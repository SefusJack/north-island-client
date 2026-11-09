#ifndef NORTH_ISLAND_CLIENT_RULES_PATH_FINDER_H
#define NORTH_ISLAND_CLIENT_RULES_PATH_FINDER_H

#include "client/models/world/WorldMap.h"
#include "core/Point.h"

#include <functional>
#include <vector>

namespace client {

    /// @brief Finds the way between two map points over the map's 20 x 20 cells.
    /// @details Original: FindMovePath.
    /// @param map The world map.
    /// @param from Start map point.
    /// @param to Destination map point.
    /// @param walkClass Which walk class decides walkability.
    /// @return Corners to walk to, the last the destination, each on its cell's anchor; empty to stay.
    std::vector<core::Point> findPath(const WorldMap& map, core::Point from, core::Point to, int walkClass = 0);

    /// @brief Whether a line between two cells meets no blocked cell.
    /// @details Original: IsLineWalkable / IsLineReachable test. Both cells beside the line (ceil and floor) must be free, so a line cannot squeeze between two blocked corners.
    /// @param fromCell Start cell.
    /// @param toCell End cell.
    /// @param blocked Tells whether a cell is blocked.
    /// @return True when the line is clear.
    bool isLineClear(core::Point fromCell, core::Point toCell, const std::function<bool(core::Point)>& blocked);

    /// @brief Whether every cell on the line between two cells is walkable.
    /// @details Original: IsLineWalkable.
    /// @param map The world map.
    /// @param fromCell Start cell.
    /// @param toCell End cell.
    /// @param walkClass Which walk class decides walkability.
    /// @return True when walkable.
    bool isLineWalkable(const WorldMap& map, core::Point fromCell, core::Point toCell, int walkClass = 0);

}

#endif
