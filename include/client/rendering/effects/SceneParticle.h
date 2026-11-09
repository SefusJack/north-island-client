#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_SCENE_PARTICLE_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_SCENE_PARTICLE_H

#include "core/Point.h"

#include <cstdint>

namespace client {

    /// @brief One leaf, steam puff, drop, bubble, ribbon or star of the scene effects.
    /// @details Original: TLeaf, TSteam, TRain, TBubble, TRibbon, TStar. It moves over map points from where it starts towards a target,
    /// turning frame by frame, and may sway sideways.
    struct SceneParticle {
        bool active = false;
        bool arrived = false;
        int picture = 0;
        int frame = 0;
        int speedLevel = 2;
        std::uint64_t tickMs = 0;
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
