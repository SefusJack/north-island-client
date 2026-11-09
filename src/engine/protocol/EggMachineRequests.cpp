#include "protocol/EggMachineRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kHouse = 62; // action 62: house / furniture

        /// @brief Starts a client 62/sub message with the u16 piece.
        /// @param sub Sub id.
        /// @param piece Furniture piece.
        /// @return A writer holding the message head.
        core::ByteWriter head(std::uint8_t sub, int piece) {
            core::ByteWriter writer;
            writer.u8(kHouse).u8(sub).u16(static_cast<std::uint16_t>(piece));
            return writer;
        }

    }

    core::Bytes putEggStackRequest(int piece, int bagSlot, int place) {
        core::ByteWriter writer = head(31, piece); // 62/31: put a stack in the machine
        writer.u8(static_cast<std::uint8_t>(bagSlot)).u8(static_cast<std::uint8_t>(place));
        return writer.take();
    }

    core::Bytes setEggPriceRequest(int piece, std::int32_t price) {
        core::ByteWriter writer = head(32, piece); // 62/32: set the play price
        writer.i32(price);
        return writer.take();
    }

    core::Bytes playEggMachineRequest(std::int32_t ownerId, int piece, std::int32_t price) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(33).i32(ownerId).u16(static_cast<std::uint16_t>(piece)).i32(price); // 62/33: play the machine
        return writer.take();
    }

    core::Bytes feedEggMachineRequest(int bagSlot) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(34).u8(static_cast<std::uint8_t>(bagSlot)); // 62/34: feed a Token Egg
        return writer.take();
    }

    core::Bytes retakeEggStackRequest(int piece, int place) {
        core::ByteWriter writer = head(35, piece); // 62/35: take a stack back
        writer.u8(static_cast<std::uint8_t>(place));
        return writer.take();
    }

    core::Bytes askEggMachineRequest(int piece, std::int32_t ownerId) {
        core::ByteWriter writer = head(36, piece); // 62/36: ask for the machine's price and eggs
        writer.i32(ownerId);
        return writer.take();
    }

}
