#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_SCREEN_EFFECTS_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_SCREEN_EFFECTS_H

#include "core/Point.h"

namespace client {

    /// @brief What a skill's motion does to the whole battle screen.
    /// @details Original: TFightField.FShaking, FShake, FFading, FFadeLevel. The shake moves the fighters, the effect layers
    /// and the ground's layers, the fade darkens the ground.
    struct ScreenEffects {
        bool shaking = false;
        core::Point shake;
        bool fading = false;
        int fadeLevel = 255; // 255: no fade at all
    };

}

#endif
