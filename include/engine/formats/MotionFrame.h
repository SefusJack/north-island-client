#ifndef NORTH_ISLAND_FORMATS_MOTION_FRAME_H
#define NORTH_ISLAND_FORMATS_MOTION_FRAME_H

namespace formats {

    /// @brief One step of a fighter's motion.
    /// @details Original: SkillData.MBTM; the sprite slot shown (the right-hand side mirrors it), the frame held in it
    /// (or 0 to play it), the curve of the way there and how long the step lasts.
    struct MotionFrame {
        int slot = 0;
        int hold = 0;
        double slope = 0;
        int durationMs = 0;
    };

}

#endif
