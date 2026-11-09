#ifndef NORTH_ISLAND_FORMATS_MOTION_CAMERA_STEP_H
#define NORTH_ISLAND_FORMATS_MOTION_CAMERA_STEP_H

#include "core/Point.h"

namespace formats {

    /// @brief Where the battle's camera goes during a motion.
    /// @details The point, its speed, when the step ends (a step of the motion below 101, else milliseconds)
    /// and what it follows (0 nothing, 1 the fighter acting, 2 the target, 3.. an effect layer).
    struct MotionCameraStep {
        core::Point point;
        double speed = 0;
        int until = 0;
        int focus = 0;
    };

}

#endif
