#include "protocol/ReferRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kItems = 23; // action 23: items
        constexpr std::uint8_t kRefer = 103; // 23/103: search an item for its materials
        constexpr std::uint8_t kWithGlass = 1;

    }

    core::Bytes referSearchRequest(int itemId, int itemSlot, int glassSlot) {
        core::ByteWriter writer;
        writer.u8(kItems)
            .u8(kRefer)
            .u8(kWithGlass)
            .u16(static_cast<std::uint16_t>(itemId))
            .u8(static_cast<std::uint8_t>(itemSlot))
            .u8(static_cast<std::uint8_t>(glassSlot));
        return writer.take();
    }

}
