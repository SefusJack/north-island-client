#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_SNOW_FLAKE_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_SNOW_FLAKE_H

#include "core/Point.h"

#include <cstdint>

namespace client {

    /// @brief One flake of the scene snow.
    /// @details Original: TSnow. It falls over map points, shaking sideways, and its picture (one of three, four frames stacked) turns
    /// frame by frame.
    struct SnowFlake {
        bool active = false;
        bool arrived = false;
        int picture = 0;
        int frame = 0;
        int speedLevel = 2;
        std::uint64_t frameMs = 0;
        int frameDelay = 0;
        core::Point position;
        core::Point target;
        int shakeRange = 0;
        bool shakeRight = true;
        int shakePhase = 0;
        int shakeOffset = 0;
    };

}

#endif
