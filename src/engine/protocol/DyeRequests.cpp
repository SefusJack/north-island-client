#include "protocol/DyeRequests.h"

namespace protocol {

    core::Bytes dyeRequest(int bagSlot, int red, int green, int blue) {
        return {
            23, // action 23: items
            38, // sub 38: dye
            static_cast<std::uint8_t>(bagSlot),
            static_cast<std::uint8_t>(red),
            static_cast<std::uint8_t>(green),
            static_cast<std::uint8_t>(blue)
        }; // 23/38: dye colour (action 23)
    }

}
