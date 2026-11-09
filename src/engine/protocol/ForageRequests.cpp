#include "protocol/ForageRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kItems = 23; // action 23: items

    }

    core::Bytes forageStartRequest(
        int boneSlot,
        int petSlot,
        int itemId,
        int targets,
        int ordinal
    ) {
        core::ByteWriter writer;
        writer.u8(kItems)
            .u8(88) // 23/88: start foraging
            .u8(static_cast<std::uint8_t>(boneSlot))
            .u8(static_cast<std::uint8_t>(petSlot))
            .u16(static_cast<std::uint16_t>(itemId))
            .u8(static_cast<std::uint8_t>(targets))
            .u8(static_cast<std::uint8_t>(ordinal));
        return writer.take();
    }

    core::Bytes forageCancelRequest() { return {kItems, 91}; } // 23/91: stop foraging

}
