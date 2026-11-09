#ifndef NORTH_ISLAND_CLIENT_SERVICES_FRAME_VERTICAL_BLANK_H
#define NORTH_ISLAND_CLIENT_SERVICES_FRAME_VERTICAL_BLANK_H

#include <cmath>
#include <cstdint>

namespace client {

    /// @brief Models how long the original's frame waited for the screen's vertical blank.
    /// @details Original: DXDraw1 has doWaitVBlank in its Options and the window is not full screen, so TDXDrawDriverBlt.Flip (last call of DXTimer1Timer) calls WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN) before it blits; FRefreshRate is 60 when the monitor does not say (DXDraw1Initialize).
    class VerticalBlank {
        public:
            static constexpr int kDefaultRefreshHz = 60; // Hz assumed when the monitor does not say

            /// @brief Time from now to the next vertical blank.
            /// @param nowMs Current time in milliseconds.
            /// @param refreshHz Monitor refresh rate (0 or less uses kDefaultRefreshHz).
            /// @return Milliseconds to wait.
            static std::uint64_t waitMs(std::uint64_t nowMs, int refreshHz) {
                const double period = 1000.0 / (refreshHz > 0 ? refreshHz : kDefaultRefreshHz); // 1000 ms per second
                // 1e-9 is a rounding tolerance for the ceil
                const double blank = std::ceil(static_cast<double>(nowMs) / period - 1e-9) * period;
                const auto next = static_cast<std::uint64_t>(std::ceil(blank - 1e-9));
                return next > nowMs ? next - nowMs : 0;
            }
    };

}

#endif
