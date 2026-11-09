#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_PUMPKIN_GHOST_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_PUMPKIN_GHOST_H

#include "client/subsystems/minigames/HumanFrame.h"
#include "core/Point.h"

namespace client {

    /// @brief One ghost of the pumpkin game.
    /// @details Original: LD_MiniGame.pas TGhost; rises from the bottom, turns at the top, comes down; reaching the bottom costs a heart.
    struct PumpkinGhost {
        /// @brief What the last update drew of the ghost.
        enum class Shown { Nothing, Walking, Grave };

        bool active = false;
        bool hit = false;
        bool descending = false;
        core::Point pos{0, 600}; // x, y in px; y 600 is the bottom edge of the 800 x 600 field
        int slot = 4; // picture slot of the ghost (starts at 4)
        HumanFrame anim;
        Shown shown = Shown::Nothing;
        core::Point shownAt;
        int shownSlot = 0;
        int shownFrame = 0;
        bool shownCoin = false;
        Shown previousShown = Shown::Nothing;
        core::Point previousShownAt;
    };

}

#endif
