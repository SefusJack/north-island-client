#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_HUNTER_ROLE_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_HUNTER_ROLE_H

#include "client/subsystems/minigames/HumanFrame.h"
#include "core/Point.h"
#include "formats/NpcInfo.h"

#include <cstdint>

namespace client {

    /// @brief An animal of the hunting game that runs about, is shot at and attacks the player.
    /// @details Original: TH_Hunter.pas THunter_role.
    struct HunterRole {
        formats::NpcInfo npc;
        int life = 0;
        int maxLife = 0;
        bool killed = false;
        int lifeCounter = 0;
        int frameCounter = 10; // legs to run: frames left before the next move step
        bool attacking = false;
        int deathCounter = 5; // attack steps left (starts at the full 5)
        core::Point pos;
        core::Point dest;
        core::Point screen;
        core::Point previousScreen;
        int direction = 0;
        int drawScale = 10; // draw size in tenths: 10 = its own size, 20 = twice
        bool moveDone = true;
        std::uint64_t moveTickMs = 0;
        bool mouseOver = false;
        bool attackShown = false;
        int drawnSlot = 0;
        int drawnFrame = 0;
        int drawnScale = 10; // draw size last drawn, in tenths (10 = its own size)
        int drawnHeight = 0;
        core::Point drawnAt;
        HumanFrame anim;
    };

}

#endif
