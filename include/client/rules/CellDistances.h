#ifndef NORTH_ISLAND_CLIENT_RULES_CELL_DISTANCES_H
#define NORTH_ISLAND_CLIENT_RULES_CELL_DISTANCES_H

#include "client/models/world/WorldMap.h"
#include "core/Point.h"

#include <array>
#include <vector>

namespace client {

    /// @brief The path finder's distance map: cell steps from a start cell to each cell within 40 cells of it.
    class CellDistances {
        public:
            static constexpr int kReach = 40; // cells from the start the map reaches in each direction
            static constexpr int kUnreached = -1;

            /// @brief Floods the window around the start over walkable cells (left, right, up, down).
            /// @param map The map to walk on.
            /// @param start Start cell.
            /// @param walkClass Walk class used for the walkability test.
            CellDistances(
                const WorldMap& map,
                core::Point start,
                int walkClass = 0
            );

            /// @brief Tells whether a cell lies inside the 81 x 81 window around the start.
            /// @param cell Cell to test.
            /// @return True when inside the window.
            bool inWindow(core::Point cell) const;

            /// @brief Steps from the start to a cell.
            /// @param cell Cell to look up.
            /// @return The step count, kUnreached for cells that cannot be reached or lie outside the window.
            int at(core::Point cell) const;

            /// @brief Lists the cells from the start to a reached cell, each a step nearer to it.
            /// @param cell A reached cell.
            /// @return The cells in walking order, start first.
            std::vector<core::Point> trace(core::Point cell) const;

        private:
            static constexpr int kWindow = 2 * kReach + 1;

            /// @brief Index of a cell in the step array.
            /// @param cell A cell inside the window.
            /// @return Row-major index.
            std::size_t index(core::Point cell) const;

            core::Point m_origin;
            std::array<int, kWindow * kWindow> m_steps{};
    };

}

#endif
