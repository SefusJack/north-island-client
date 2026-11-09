#include "protocol/DismantleRequests.h"

namespace protocol {

    core::Bytes dismantleRequest(int pieceSlot, int pliersSlot, int mode) {
        return {
            23, // action 23: items
            86, // sub 86: dismantle a stone
            static_cast<std::uint8_t>(pieceSlot),
            static_cast<std::uint8_t>(pliersSlot),
            static_cast<std::uint8_t>(mode)
        }; // 23/86: take the stone out with Tear Pliers (action 23)
    }

}
