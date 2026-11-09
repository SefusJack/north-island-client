#include "protocol/ConfuseStoneRequests.h"

namespace protocol {

    core::Bytes confuseStoneRequest(int equipSlot, int stoneSlot) {
        return {23, 127, static_cast<std::uint8_t>(equipSlot), static_cast<std::uint8_t>(stoneSlot)}; // 23/127: raise the mounted stone (action 23)
    }

}
