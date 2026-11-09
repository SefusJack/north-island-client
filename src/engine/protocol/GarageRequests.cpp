#include "protocol/GarageRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kHouse = 62; // action 62: house / furniture

    }

    core::Bytes openGarageRequest(int piece) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(51).u16(static_cast<std::uint16_t>(piece)); // 62/51: the Garage was clicked
        return writer.take();
    }

    core::Bytes putInGarageRequest(int piece, int bagSlot, int cell) {
        core::ByteWriter writer;
        writer.u8(kHouse)
            .u8(47) // sub 47: park a vehicle
            .u16(static_cast<std::uint16_t>(piece))
            .u8(static_cast<std::uint8_t>(bagSlot))
            .u8(static_cast<std::uint8_t>(cell)); // 62/47: park a vehicle
        return writer.take();
    }

    core::Bytes takeFromGarageRequest(int piece, int cell) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(48).u16(static_cast<std::uint16_t>(piece)).u8(static_cast<std::uint8_t>(cell)); // 62/48: take a vehicle back
        return writer.take();
    }

}
