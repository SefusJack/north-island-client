#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_SCOTD_ROLE_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_SCOTD_ROLE_H

#include "client/subsystems/minigames/HumanFrame.h"
#include "core/Point.h"

#include <cstdint>

namespace client {

    /// @brief A figure of the Scotd game: the player's own, or an NPC that jumps about the field.
    /// @details Original: TH_scotd.pas TScotd_role (a THuman); fields keep the original names.
    struct ScotdRole {
        core::Point pos;
        core::Point screen;
        core::Point dest;
        int direction = 0;
        int curAction = 0;
        int holdFrame = HumanFrame::kAnimate;
        bool caught = false;
        bool inJump = false;
        bool attacking = false;
        bool hitStun = false;
        bool hitFlash = false;
        bool moving = false;
        int animPhase = 0;
        int jumpPower = 0;
        int jumpCounter = 0;
        int despawnCountdown = 3; // steps left before the role is removed
        int hitStunCounter = 100; // ticks the hit stun lasts
        int hitFlashCounter = 50; // ticks the hit flash lasts
        int moveSteps = 10; // steps left in the current move
        std::uint64_t moveTickMs = 0;
        HumanFrame anim;
        int drawnSlot = 0;
        int drawnFrame = 0;
        core::Point drawnAt;
        core::Point shadowAt;
        core::Point previousDrawnAt;
        core::Point previousShadowAt;
    };

}

#endif
