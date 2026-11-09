#ifndef NORTH_ISLAND_FORMATS_MOTION_LAYER_H
#define NORTH_ISLAND_FORMATS_MOTION_LAYER_H

#include "core/Point.h"

#include <vector>

namespace formats {

    /// @brief An effect picture a motion plays.
    /// @details Original: CH_TLight; the effect ("<id>_<direction>" pictures), the points it travels (from the fighter,
    /// mirrored on the right-hand side), its frames and their pace, when it starts (a step of the motion below 101, else milliseconds) and how it moves.
    struct MotionLayer {
        int effectId = 0;
        std::vector<core::Point> points;
        double slope = 0;
        int frameDelayMs = 0;
        int frameCount = 0;
        int startFrame = 0;
        int endFrame = 0;
        int value = 0;
        int startDelay = 0;
        int playMode = 0;
        int moveMode = 0;
        int repeatCount = 0;
        int arcDirection = 0;
        int direction = 0;
        float arcHeight = 0;
    };

}

#endif
