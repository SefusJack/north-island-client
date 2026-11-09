#include "protocol/FishingRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kItems = 23; // action 23: items

    }

    core::Bytes startFishingRequest(int rodSlot, int rodKind) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(53).u8(static_cast<std::uint8_t>(rodSlot)).u8(static_cast<std::uint8_t>(rodKind)); // 23/53: start fishing
        return writer.take();
    }

    core::Bytes stopFishingRequest() {
        core::ByteWriter writer;
        writer.u8(kItems).u8(54); // 23/54: stop fishing
        return writer.take();
    }

}
