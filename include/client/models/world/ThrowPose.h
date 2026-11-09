#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_THROW_POSE_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_THROW_POSE_H

#include <cstdint>

namespace client {

    /// @brief The throwing pose: the thrower plays the throwing action until the stop frame, then stands again facing the way he threw.
    /// @details Original: FSavedActionKind 2 (HandleThrowFxPacket); stop frame is FAnimStopFrame.
    struct ThrowPose {
        std::uint64_t endsMs = 0;
        // direction 4 is south
        int facing = 4;

        /// @brief Tells whether the pose is over.
        /// @details Over when the frame counter reaches FAnimStopFrame, or (a safety stop) its time ran out or the action was cleared.
        /// @param frame Current frame counter.
        /// @param nowMs Current time in ms.
        /// @param action Current action (0: cleared).
        /// @return True when over.
        bool finished(int frame, std::uint64_t nowMs, int action) const { return frame >= kStopFrame || nowMs >= endsMs || action == 0; }

        static constexpr int kStopFrame = 3;
    };

}

#endif
