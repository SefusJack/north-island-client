#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_THROW_THROWER_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_THROW_THROWER_H

#include "client/subsystems/minigames/HumanFrame.h"
#include "core/Point.h"

namespace client {

    /// @brief One person of the throwing game: position, destination, action and frame.
    /// @details Original: CH_TThrowGame humans and rival throwers; FDirection 16/17 standing, 34/35 winding up, 64/65 running, 30/31 hit, 26/27 down; hold frame 255 means it runs.
    struct ThrowThrower {
        core::Point screen;
        core::Point previousScreen;
        core::Point dest;
        int direction = 16; // pose/direction code 16: standing
        int holdFrame = 255; // 255: no held frame (keep animating)
        int throwSide = 1;
        HumanFrame anim;
    };

}

#endif
