#include "protocol/PillarRequests.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kHouse = 65; // action 65: tents and house floor pillars
        constexpr std::uint8_t kPillars = 11; // 65/11: pillar changes of a house floor

    }

    core::Bytes pillarChangesRequest(const core::Bytes& body) {
        core::Bytes message{kHouse, kPillars};
        message.insert(message.end(), body.begin(), body.end());
        return message;
    }

}
