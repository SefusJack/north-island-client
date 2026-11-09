#include "protocol/PackingRequests.h"

namespace protocol {

    core::Bytes packingRequest(int boxSlot, int pieceSlot) {
        return {23, 112, 1, static_cast<std::uint8_t>(boxSlot), static_cast<std::uint8_t>(pieceSlot)}; // action 23, sub 112, mode 1: pack the piece
    }

    core::Bytes openPackedBoxRequest(int boxSlot) {
        return {23, 112, 2, static_cast<std::uint8_t>(boxSlot), 0}; // action 23, sub 112, mode 2: take the piece out
    }

}
