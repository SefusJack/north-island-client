#include "protocol/ReinforceRequests.h"

namespace protocol {

    core::Bytes reinforceRequest(int scrollSlot, int equipSlot) {
        return {23, 121, static_cast<std::uint8_t>(scrollSlot), static_cast<std::uint8_t>(equipSlot)}; // action 23, sub 121: fusion scroll
    }

}
