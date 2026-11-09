#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_DOOR_LIGHT_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_DOOR_LIGHT_H

#include "core/Point.h"
#include "gfx/PictureId.h"

#include <cstdint>

namespace client {

    /// @brief The glow over a door: a strip of frames added to the screen as light, next frame every 300 ms.
    /// @details Original: TCY_DoorLight ("DoorLight" 13 frames, "SmallDoorLight" 4 frames).
    struct DoorLight {
        gfx::PictureId picture;
        core::Point position;
        int frames = 1;
        int frame = 0;
        std::uint64_t frameMs = 0;
    };

}

#endif
