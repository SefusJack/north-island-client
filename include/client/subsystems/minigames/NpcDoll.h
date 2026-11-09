#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_NPC_DOLL_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_NPC_DOLL_H

#include "client/subsystems/minigames/HumanFrame.h"
#include "core/Point.h"
#include "formats/NpcInfo.h"

namespace client {

    /// @brief A doll of the claw machine: an NPC kind standing on the machine's floor.
    /// @details Original: MZ_Doll.pas TNpcDoll; faces left or right (directions 11 and 13; 30 and 31 once the claw has it); screen is the place of its feet.
    struct NpcDoll {
        formats::NpcInfo npc;
        int kindIdx = 0;
        int direction = 11; // facing: 11 left, 13 right (30 and 31 once the claw has it)
        core::Point screen{-100, -100}; // off screen until placed, px
        core::Point previousScreen{-100, -100}; // off screen until placed, px
        HumanFrame anim{1, 0};
    };

}

#endif
