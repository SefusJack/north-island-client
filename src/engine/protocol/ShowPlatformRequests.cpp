#include "protocol/ShowPlatformRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kHouse = 62; // action 62: house / furniture

    }

    core::Bytes openShowPlatformRequest(int piece) {
        core::ByteWriter writer;
        // 55 = open show platform sub id
        writer.u8(kHouse).u8(55).u16(static_cast<std::uint16_t>(piece));
        return writer.take();
    }

    core::Bytes showOnPlatformRequest(int piece, int cell) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(49).u16(static_cast<std::uint16_t>(piece)).i32(cell); // 49 = exhibit slot sub id
        return writer.take();
    }

    core::Bytes takeOffPlatformRequest(int cell) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(49).u16(static_cast<std::uint16_t>(cell)).i32(0).i32(0); // 49 = exhibit slot sub id
        return writer.take();
    }

}
