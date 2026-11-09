#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_ROOM_RECT_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_ROOM_RECT_H

namespace client {

    /// @brief The cell box a house room covers, edges included.
    /// @details Original: TFurnRoomRec.
    struct HouseRoomRect {
        int left = 0;
        int top = 0;
        int right = 0;
        int bottom = 0;

        /// @brief Tells whether a cell is inside the box.
        /// @details Original: PointInBox.
        /// @param cellX Cell column.
        /// @param cellY Cell row.
        /// @return True when inside (edges included).
        bool contains(int cellX, int cellY) const;

        /// @brief The box of a floor room of a size.
        /// @details Original: GetRoomSizeRect.
        /// @param size Room size 1, 2 or 3.
        /// @return The box, empty for other sizes.
        static HouseRoomRect ofSize(int size);

        /// @brief The width of the box in cells.
        /// @details Original: GetRectSpanX.
        /// @return Cell count across.
        int spanX() const;

        /// @brief The depth of the box in cells.
        /// @details Original: GetRectSpanY.
        /// @return Cell count deep.
        int spanY() const;
    };

}

#endif
