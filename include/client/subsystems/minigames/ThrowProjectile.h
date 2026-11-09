#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_THROW_PROJECTILE_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_THROW_PROJECTILE_H

#include "client/rendering/effects/LightEffect.h"
#include "core/Point.h"

#include <cstdint>

namespace client {

    /// @brief A ball of the throwing game: position, target, arc and state.
    /// @details Original: TThrowProjectile; Active 0 free, 1 flying, 2 hit a person, 3 landed.
    struct ThrowProjectile {
        core::Point at;
        core::Point target;
        int distX = 0;
        int distY = 0;
        int distance = 0;
        int arcHeight = 0;
        core::Point previousAt;
        int previousArcHeight = 0;
        int active = 0;
        int phase = 0;
        std::uint64_t moveTickX = 0;
        std::uint64_t moveTickY = 0;
        LightEffect light;
    };

}

#endif
