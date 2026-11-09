#ifndef NORTH_ISLAND_CLIENT_MODELS_TRAVEL_VEHICLE_H
#define NORTH_ISLAND_CLIENT_MODELS_TRAVEL_VEHICLE_H

#include "client/models/character/FigureAnimation.h"
#include "client/rendering/effects/LightEffect.h"
#include "core/Point.h"

#include <cstdint>

namespace client {

    /// @brief A vehicle standing on the map: whose it is, which item, the sprite its item looks like (the front layer is the next one), where it stands
    /// and the bag place of its owner it came from.
    /// @details Original: a placed CH_TTrafficTool; the bag place is FToolSlot.
    struct Vehicle {
        int slot = 0;
        std::int32_t ownerId = 0;
        int itemId = 0;
        int kind = 0;
        int lookId = 0;
        core::Point position;
        int bagSlot = 0;
        LightEffect flight;
        FigureAnimation animation;
        int shownFrame = 0;
    };

}

#endif
