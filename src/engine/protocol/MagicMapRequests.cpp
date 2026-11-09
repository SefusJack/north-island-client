#include "protocol/MagicMapRequests.h"

namespace protocol {

    core::Bytes magicMapTeleportRequest(int place, int bagSlot, int mode) {
        return {
            23, // action 23: items
            89, // sub 89: magic map teleport
            static_cast<std::uint8_t>(place),
            static_cast<std::uint8_t>(bagSlot),
            static_cast<std::uint8_t>(mode)
        }; // action 23, sub 89: teleport item
    }

}
