#ifndef NORTH_ISLAND_CLIENT_SERVICES_FRAME_FRAME_COST_H
#define NORTH_ISLAND_CLIENT_SERVICES_FRAME_FRAME_COST_H

#include <cstdint>

namespace client {

    /// @brief Pads a frame to the cost the original's drawing and blit had (about 14 ms before the vertical blank).
    /// @details Original: DXTimer1Timer drew with DelphiX software blits and TDXDrawDriverBlt.Flip blitted after the blank; measured in playtest 3 C21 (Fire Wave Attack, 2026-10-26) on that machine, not a number of its code.
    class FrameCost {
        public:
            static constexpr std::uint64_t kOriginalDrawMs = 14; // ms the original took to draw a frame (measured)

            /// @brief Time a frame waits before the flip so the whole costs what the original's did.
            /// @param drawMs Milliseconds the frame took to draw and present.
            /// @return Milliseconds to wait, 0 when the frame was already as slow.
            static std::uint64_t padMs(std::uint64_t drawMs) { return drawMs >= kOriginalDrawMs ? 0 : kOriginalDrawMs - drawMs; }
    };

}

#endif
