#ifndef NORTH_ISLAND_CLIENT_MODELS_TRAVEL_VEHICLE_WRECK_H
#define NORTH_ISLAND_CLIENT_MODELS_TRAVEL_VEHICLE_WRECK_H

#include "client/rendering/effects/LightEffect.h"
#include "core/Point.h"
#include "gfx/PictureId.h"

#include <cstdint>

namespace client {

    /// @brief A vehicle that broke: the wreck falls where its owner stood and lies there for a minute.
    /// @details Original: HandleVehicleDestroyedPacket.
    struct VehicleWreck {
        int itemId = 0;
        gfx::PictureId picture;
        core::Point position;
        std::int32_t ownerId = 0;
        std::uint64_t startMs = 0;
        LightEffect fall;
    };

}

#endif
