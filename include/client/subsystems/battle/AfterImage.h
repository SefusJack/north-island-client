#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_AFTER_IMAGE_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_AFTER_IMAGE_H

#include "core/Point.h"

namespace client {

    /// @brief A ghost of a casting fighter left on the ground where it was: the pose it had, and how bright it still is; it fades a step each time it is drawn.
    /// @details Original: TAfterImage. With free frames it fades each beat, the frames between fading it on to the next.
    struct AfterImage {
        core::Point position;
        int slot = 0;
        int frame = 0;
        int alpha = 0;
        int fade = 0;
        int drawnAlpha = 0;
        bool active = false;
    };

}

#endif
