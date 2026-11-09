#ifndef NORTH_ISLAND_FORMATS_MOTION_SCREEN_EFFECT_H
#define NORTH_ISLAND_FORMATS_MOTION_SCREEN_EFFECT_H

namespace formats {

    /// @brief A whole-screen effect during a motion (shaking, fading ...).
    /// @details Its kind and when it starts and ends (steps of the motion below 101, else milliseconds).
    struct MotionScreenEffect {
        int kind = 0;
        int start = 0;
        int end = 0;
    };

}

#endif
