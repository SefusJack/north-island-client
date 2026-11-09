#ifndef NORTH_ISLAND_CLIENT_SERVICES_FRAME_FRAME_BEAT_H
#define NORTH_ISLAND_CLIENT_SERVICES_FRAME_FRAME_BEAT_H

#include <cstdint>

namespace client {

    /// @brief Computes when the next frame starts, on the 30 ms beat of the original.
    /// @details Original: TForm1 timeSetEvent 30 with FrameTimerProc posts a tick only when the last one is done (FramesDone >= FramesPosted), so a frame longer than a beat waits for the next one; one that ran 35 ms (a Sleep(35) of AdvanceMotionPath in it) is followed by the next after 60 ms.
    class FrameBeat {
        public:
            static constexpr std::uint64_t kBeatMs = 30; // ms between frame ticks (timeSetEvent 30)

            /// @brief Time to wait until the next beat after a frame.
            /// @param spentMs Milliseconds the frame took.
            /// @return Milliseconds to wait so the frame ends on a whole number of beats.
            static std::uint64_t waitMs(std::uint64_t spentMs) {
                const std::uint64_t beats = spentMs <= kBeatMs ? 1 : (spentMs + kBeatMs - 1) / kBeatMs;
                return beats * kBeatMs - spentMs;
            }
    };

}

#endif
