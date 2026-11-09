#include "protocol/MagnetRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kItems = 23; // action 23: items
        constexpr std::uint8_t kMagnet = 90; // 23/90: Magic Magnet

    }

    core::Bytes askMagnetRequest(int bagSlot, std::int32_t playerId) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(kMagnet).u8(1).u8(static_cast<std::uint8_t>(bagSlot)).i32(playerId); // magnet sub 1: ask
        return writer.take();
    }

    core::Bytes agreeMagnetRequest() {
        core::ByteWriter writer;
        writer.u8(kItems).u8(kMagnet).u8(2); // magnet sub 2: agree
        return writer.take();
    }

    core::Bytes refuseMagnetRequest() {
        core::ByteWriter writer;
        writer.u8(kItems).u8(kMagnet).u8(3); // magnet sub 3: refuse
        return writer.take();
    }

    core::Bytes cannotMagnetRequest(int reason) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(kMagnet).u8(4).u8(static_cast<std::uint8_t>(reason)); // magnet sub 4: cannot, with a reason
        return writer.take();
    }

}
