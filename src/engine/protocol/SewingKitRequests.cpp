#include "protocol/SewingKitRequests.h"

namespace protocol {

    core::Bytes sewingKitRequest(int kitSlot, int equipSlot, int poseSlot) {
        // 23 = item action, 105 = sewing kit sub id, 1 = use
        return {23, 105, 1, static_cast<std::uint8_t>(kitSlot), static_cast<std::uint8_t>(equipSlot), static_cast<std::uint8_t>(poseSlot)};
    }

}
