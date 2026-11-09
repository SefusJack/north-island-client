#include "protocol/RepairRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kItems = 23; // action 23: items
        constexpr std::uint8_t kRepair = 84; // 23/84: repair

    }

    core::Bytes repairItemRequest(int itemId, int itemSlot, int toolSlot) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(kRepair).u16(static_cast<std::uint16_t>(itemId)).u8(static_cast<std::uint8_t>(itemSlot)).u8(static_cast<std::uint8_t>(toolSlot));
        return writer.take();
    }

}
