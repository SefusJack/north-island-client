#include "protocol/GpsRequests.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kGps = 74; // action 74: GPS

        /// @brief Builds a client 74/sub request with two u8 arguments.
        /// @param sub Sub id.
        /// @param first First argument.
        /// @param second Second argument.
        /// @return The encoded request.
        core::Bytes request(std::uint8_t sub, int first, int second) {
            return {kGps, sub, static_cast<std::uint8_t>(first), static_cast<std::uint8_t>(second)};
        }

    }

    core::Bytes gpsMarkRequest(int gpsSlot, int point) { return request(1, gpsSlot, point); }

    core::Bytes gpsTeleportRequest(int gpsSlot, int point) { return request(2, gpsSlot, point); } // 74/2: go to the point

    core::Bytes gpsUpgradeRequest(int crystalSlot, int gpsSlot) { return request(4, crystalSlot, gpsSlot); } // 74/4: raise the GPS one step

    core::Bytes dreamingPillRequest() { return {kGps, 3, 1}; } // 74/3 option 1: Dreaming Pill duration

}
