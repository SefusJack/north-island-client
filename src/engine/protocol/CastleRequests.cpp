#include "protocol/CastleRequests.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kCastle = 70; // action 70: castle / Trojan War

    }

    core::Bytes trojanWarLeaveRequest() { return {kCastle, 3}; } // 70/3: leave the Trojan War battle

    core::Bytes castleTeleportRequest() { return {kCastle, 7}; } // 70/7: guild member teleports to the castle

    core::Bytes trojanWarFlagResetRequest() { return {kCastle, 4}; } // 70/4: clear the disguise flag

}
