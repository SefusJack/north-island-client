#ifndef NORTH_ISLAND_CLIENT_RULES_HOUSE_MOVE_PATH_H
#define NORTH_ISLAND_CLIENT_RULES_HOUSE_MOVE_PATH_H

#include "client/models/house/HomeFurniture.h"
#include "core/Point.h"
#include "core/Rect.h"

#include <array>
#include <vector>

namespace client {

    /// @brief Walking route search inside one floor room of a house; one object is one search.
    /// @details Original: RE_Furniture.pas FindHouseMovePath (four-direction mode) with CellPathToScreen.
    class HouseMovePath {
        public:
            /// @brief Prepares a search in one room.
            /// @param furniture The house furniture (cells and rooms).
            /// @param room The floor room index.
            /// @param view The part of the map the screen shows; walked cells must lie inside it.
            HouseMovePath(
                const HomeFurniture& furniture,
                int room,
                const core::Rect& view
            );

            /// @brief Finds the walking route between two map points.
            /// @param from Start map point.
            /// @param to Destination map point.
            /// @return Screen points (middle of each cell's floor), first the start cell; empty when the player stays.
            std::vector<core::Point> find(core::Point from, core::Point to);

        private:
            /// @brief Search state of one cell.
            /// @details Original: HousePathNodes.
            struct Node {
                int closed = 0;
                core::Point parent;
                core::Point pos;
                int g = 0;
                double h = 0;
                double cost = 0;
            };

            static constexpr int kNodes = 101; // nodes along each side of the search grid (101 x 101)
            static constexpr int kPathMost = 70; // most steps of a found path

            /// @brief Converts a room cell (from 1) to a grid cell (from 29, 27).
            /// @param cell Room cell.
            /// @return Grid cell.
            static core::Point absolute(core::Point cell);

            /// @brief Converts a grid cell to a room cell.
            /// @param cell Grid cell.
            /// @return Room cell.
            static core::Point relative(core::Point cell);

            /// @brief Search node of a room cell.
            /// @param cell Room cell.
            /// @return The node.
            Node& node(core::Point cell) { return m_nodes[static_cast<std::size_t>(cell.x * kNodes + cell.y)]; }

            /// @brief Whether a grid cell has nothing in the way.
            /// @details Original: GetHouseCell in [0, 241..248].
            /// @param cell Grid cell.
            /// @return True when walkable.
            bool free(core::Point cell) const;

            /// @brief Whether a grid cell belongs to the room.
            /// @param cell Grid cell.
            /// @return True when inside the room.
            bool inRoom(core::Point cell) const;

            /// @brief Whether a room cell lies in the room and on the screen.
            /// @details Original: Check.
            /// @param cell Room cell.
            /// @return True when usable.
            bool check(core::Point cell) const;

            /// @brief Tests whether every cell on a straight line is free.
            /// @details Original: LineOK.
            /// @param from Start grid cell.
            /// @param to End grid cell.
            /// @param last Where the line was stopped ((-1, -1) for a start that is not tried).
            /// @return True when the whole line is free.
            bool lineOk(core::Point from, core::Point to, core::Point& last) const;

            /// @brief The sort key G + H of an open node.
            /// @details Original: Compare.
            /// @param index Node place in the grid.
            /// @return G + H.
            double estimate(int index) const;

            /// @brief Sorts the open nodes by G + H.
            void sortOpen();

            /// @brief Quick sort of the open list (not stable), as the game's Classes.QuickSort.
            /// @param low First index.
            /// @param high Last index.
            void quickSort(int low, int high);

            /// @brief Opens or closes the four cells round one and closes the cell.
            /// @details Original: Expand.
            /// @param cell The cell expanded.
            /// @param start Search start cell.
            /// @param goal Search goal cell.
            void expand(core::Point cell, core::Point start, core::Point goal);

            /// @brief Takes the first open node until the goal comes up or none is left.
            /// @details Original: Pop.
            /// @param start Search start cell.
            /// @param goal Search goal cell.
            void pop(core::Point start, core::Point goal);

            /// @brief Shortens the route where a straight line serves.
            /// @details Original: Smooth.
            /// @param start The route's start cell.
            /// @return True when the route got shorter.
            bool smooth(core::Point start);

            /// @brief Searches round the furniture from one cell to another and keeps the route.
            /// @details Original: Walk.
            /// @param start Start grid cell.
            /// @param goal Goal grid cell.
            void walk(core::Point start, core::Point goal);

            /// @brief Whether the cell and the four round it are all taken or outside the room.
            /// @details Original: Search.Clear.
            /// @param cell Grid cell.
            /// @return True when boxed in.
            bool boxedIn(core::Point cell) const;

            /// @brief Builds the route: a straight line when clear, else a search.
            /// @details Original: Search.
            /// @param from Start grid cell.
            /// @param to Destination grid cell.
            void search(core::Point from, core::Point to);

            const HomeFurniture& m_furniture;
            int m_room;
            core::Rect m_view;
            std::vector<Node> m_nodes;
            std::vector<int> m_open;
            Node m_best;
            std::array<core::Point, kPathMost + 2> m_path{};
            int m_length = 0;
    };

}

#endif
