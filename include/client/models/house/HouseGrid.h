#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_GRID_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_GRID_H

#include "core/Point.h"

namespace client {

    /// @brief The cells of a house floor room: cells of 60 x 30 pixels in the isometric frame on the 1472 x 960 map.
    /// @details Original: RE_Furniture.pas TRE_FurnitureMng; DZ_Game.pas sets it up with SetupFurnitureGrid(60, 30, 1472, 960).
    struct HouseGrid {
        static constexpr int kCellWidth = 60; // width of a cell in the isometric frame, px
        static constexpr int kCellHeight = 30; // height of a cell in the isometric frame, px
        static constexpr int kCells = 57; // cells along each side of the whole grid (57 x 57)
        static constexpr core::Point kOrigin{-960 + 69, -375 - 178}; // map offset of the grid origin, px (x -960 + 69, y -375 - 178)
        static constexpr int kLevelHeight = 30; // pixels a level (z) rises above the one below
        static constexpr int kGridColumn = 29; // first grid cell column of a room (cell x - 29 + 1 is the room column)
        static constexpr int kGridRow = 27; // first grid cell row of a room (cell y - 27 + 1 is the room row)
        static constexpr int kGridWidth = 16; // columns of a room
        static constexpr int kGridDepth = 24; // rows of a room

        /// @brief A cell: column x, row y and level z.
        struct Cell {
            int x = 0;
            int y = 0;
            int z = 0;
        };

        /// @brief Where the middle of the top of a cell's floor is on the map.
        /// @details Original: CellToScreen.
        /// @param cellX Cell column.
        /// @param cellY Cell row.
        /// @param cellZ Level.
        /// @return The map point; (-1, -1) for cells and levels outside 0..1000.
        static core::Point cellToScreen(int cellX, int cellY, int cellZ);

        /// @brief The cell a map point (feet of a person) is over, on level 1.
        /// @details Original: ScreenToCell (LocateCell).
        /// @param mapPoint The map point.
        /// @return The cell.
        static Cell screenToCell(core::Point mapPoint);
    };

}

#endif
