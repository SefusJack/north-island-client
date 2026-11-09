#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_RIDDEN_VEHICLE_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_RIDDEN_VEHICLE_H

#include "client/models/character/FigureAnimation.h"

#include <cstdint>
#include <vector>

namespace client {

    /// @brief The vehicle a player rides: the item and sprite it looks like, whose it is and the seat he sits in.
    /// @details Original: FTrafficState 2, FTrafficTool, FVehicleSeat, FVehicleItemId. Seat 1 is the driver, 2..4 the passengers of the
    /// driver's team; the driver keeps the ids of the passengers in their seats (FPassengers).
    struct RiddenVehicle {
        int itemId = 0;
        int kind = 0;
        int lookId = 0;
        std::int32_t ownerId = 0;
        int seat = 1;
        std::vector<std::int32_t> passengers;
        FigureAnimation animation;
    };

}

#endif
