#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_DRAG_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_DRAG_H

#include "client/models/house/HouseGrid.h"
#include "core/Point.h"
#include "gfx/PictureId.h"

namespace client {

    /// @brief A piece of furniture being carried to another cell: where it would stand if let go and the picture it is drawn with.
    /// @details Original: Se_DragItem with a TGdUtensil as its source. The piece itself stays where it was, lit, until the drop is answered.
    struct HouseDrag {
        int furnIndex = 0;
        int slot = 0;
        int itemId = 0;
        int direction = 0;
        HouseGrid::Cell drop;
        gfx::PictureId picture;
        gfx::PictureId turned;
        core::Point size;
        core::Point grab;
        core::Point click;
    };

}

#endif
