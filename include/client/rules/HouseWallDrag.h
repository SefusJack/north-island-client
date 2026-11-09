#ifndef NORTH_ISLAND_CLIENT_RULES_HOUSE_WALL_DRAG_H
#define NORTH_ISLAND_CLIENT_RULES_HOUSE_WALL_DRAG_H

#include "client/models/house/HouseGrid.h"
#include "client/models/house/HouseInterior.h"
#include "client/models/world/Camera.h"
#include "core/Point.h"
#include "formats/ItemTable.h"

#include <optional>

namespace client {

    /// @brief Finds where a wall piece carried over a house lands.
    /// @details Original: se_item.pas HitTestDragWallCell and the wall part of CalcDragPlaceCell.
    class HouseWallDrag {
        public:
            /// @brief The cell a wall piece would hang at and the wall it would face.
            struct Place {
                HouseGrid::Cell cell;
                int direction = 0;
            };

            /// @brief Binds the finder to a house and the item table.
            /// @param interior The house interior being edited.
            /// @param items The item table.
            HouseWallDrag(
                const HouseInterior& interior,
                const formats::ItemTable& items
            )
                : m_interior(interior),
                  m_items(items) {}

            /// @brief Finds the wall cell or pillar cell under the mouse, front ones first.
            /// @details Original: HitTestDragWallCell. Cells are in the room's own grid (column 0 and the room's top row are the walls).
            /// @param mouse Mouse position on the screen.
            /// @param camera Camera used to turn the mouse into a map point.
            /// @return The cell, or (0, 0) for none.
            core::Point hitCell(core::Point mouse, const Camera& camera) const;

            /// @brief Finds where a carried wall-flag piece hangs.
            /// @details Original: CalcDragPlaceCell for a piece with the on-wall flag.
            /// @param itemId The carried item.
            /// @param rowCol Row/column of the cell under the piece's middle.
            /// @param under The cell under the piece's middle (kept unless the mouse is over a wall or a pillar).
            /// @param mapMouse Mouse position on the map.
            /// @return The place, or nothing for an item with no wall level.
            std::optional<Place> place(int itemId, core::Point rowCol, const HouseGrid::Cell& under, core::Point mapMouse) const;

        private:
            /// @brief Whether the mouse is over a wall cell's picture.
            /// @details Original: IsMouseOnWallCell. The left wall's cell uses the wall item's second facing, the back wall's its first.
            /// @param column Cell column in the room's grid.
            /// @param row Cell row in the room's grid.
            /// @param mapMouse Mouse position on the map.
            /// @return True when over an opaque pixel.
            bool onWallCell(int column, int row, core::Point mapMouse) const;

            /// @brief Whether the mouse is over a pillar's picture.
            /// @param column Cell column in the room's grid.
            /// @param row Cell row in the room's grid.
            /// @param mapMouse Mouse position on the map.
            /// @return True when over an opaque pixel.
            bool onPillar(int column, int row, core::Point mapMouse) const;

            const HouseInterior& m_interior;
            const formats::ItemTable& m_items;
    };

}

#endif
