#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_HUMAN_FRAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_HUMAN_FRAME_H

#include <cstdint>

namespace client {

    /// @brief The animation frame a human shows in its action (original DrawRoleSprite taking it from NextAnimFrame), stepped by an interval.
    struct HumanFrame {
        static constexpr std::uint64_t kIntervalMs = 230; // milliseconds per frame while standing
        static constexpr std::uint64_t kMovingIntervalMs = 100; // milliseconds per frame while moving
        static constexpr int kAnimate = 255; // hold-frame value meaning "keep animating" (255 = no held frame)

        int frame = 0;
        std::uint64_t lastStepMs = 0;

        /// @brief Goes on to the next frame when more than the interval has passed (original NextAnimFrame with CheckAnimInterval).
        /// @param nowMs Current time in ms.
        /// @param frameCount Frames in the action.
        /// @param holdLast Stay on the last frame instead of going back to the first (actions of the group of 26: lying down).
        /// @param intervalMs Interval between steps in ms.
        /// @param fixedFrame Frame drawn instead (the human's FHoldFrame) unless it is 255.
        /// @return The frame to draw now.
        int next(
            std::uint64_t nowMs,
            int frameCount,
            bool holdLast = false,
            std::uint64_t intervalMs = kIntervalMs,
            int fixedFrame = kAnimate
        ) {
            if (nowMs - lastStepMs > intervalMs) {
                lastStepMs = nowMs;
                ++frame;
            }
            if (frame >= frameCount) {
                frame = holdLast && frameCount > 0 ? frameCount - 1 : 0;
            }
            if (fixedFrame != kAnimate) {
                frame = fixedFrame;
            }
            return frame;
        }
    };

}

#endif
