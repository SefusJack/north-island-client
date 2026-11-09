#include "protocol/SystemRequests.h"

namespace protocol {

    core::Bytes switchRequest(int sub, int state) {
        return {33, static_cast<std::uint8_t>(sub), static_cast<std::uint8_t>(state)}; // 33 = system settings action
    }

    core::Bytes talkFormReadyRequest() { return {23, 77}; } // 23 = item action, 77 = talk form ready sub id

    core::Bytes channelsRequest(int bits) { return {33, 3, static_cast<std::uint8_t>(bits)}; } // 33 = system settings action, 3 = chat channels sub id

    core::Bytes returnToSpotRequest(int spot) { return {5, 17, static_cast<std::uint8_t>(spot)}; } // 5 = player action, 17 = return to revival spot sub id

    core::Bytes balanceRequest() { return {34, 1, 0}; } // 34 = account action, 1 = balance sub id

    core::Bytes stopExpressRequest() { return {23, 52}; } // 52 = stop express sub id

    core::Bytes leaveWaterRequest() { return {23, 52}; } // 52 = leave water sub id

    core::Bytes stopCollectingRequest() { return {23, 54}; } // 54 = stop collecting sub id

}
