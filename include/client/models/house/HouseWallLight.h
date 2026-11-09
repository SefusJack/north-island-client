#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_WALL_LIGHT_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_WALL_LIGHT_H

#include "core/Point.h"

namespace client {

    /// @brief What a carried wall piece lights up: the wall cell or pillar under the mouse, the piece and the facing it would have.
    /// @details Original: Se_DragItem.FCellRowCol and FDirection with FPlaceableDrag. The wall pieces under the piece and the pillars it covers are drawn lit.
    struct HouseWallLight {
        core::Point rowCol;
        int itemId = 0;
        int direction = 0;
    };

}

#endif
