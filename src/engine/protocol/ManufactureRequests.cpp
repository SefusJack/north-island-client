#include "protocol/ManufactureRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kManufacture = 64; // action 64: manufacture (crafting)

        /// @brief Writes the pet that makes it: its place and id (0, 0: the player makes it himself).
        /// @param writer Packet writer.
        /// @param petSlot Pet place (u8).
        /// @param petId Pet id (i32).
        void pet(core::ByteWriter& writer, int petSlot, int petId) { writer.u8(static_cast<std::uint8_t>(petSlot)).i32(petId); }

    }

    core::Bytes startMakingRequest(
        int piece,
        int recipe,
        const core::Bytes& materials,
        int blueprintSource,
        int blueprintSlot,
        int petSlot,
        int petId
    ) {
        core::ByteWriter writer;
        // sub 1: start making
        writer.u8(kManufacture).u8(1).u16(static_cast<std::uint16_t>(piece)).u16(static_cast<std::uint16_t>(recipe)).bytes(materials);
        writer.u8(static_cast<std::uint8_t>(blueprintSource)).u8(static_cast<std::uint8_t>(blueprintSlot));
        pet(writer, petSlot, petId);
        return writer.take();
    }

    core::Bytes continueMakingRequest(
        int piece,
        int jobSlot,
        int blueprintSource,
        int blueprintSlot,
        int petSlot,
        int petId
    ) {
        core::ByteWriter writer;
        // sub 2: continue a stopped job
        writer.u8(kManufacture).u8(2).u16(static_cast<std::uint16_t>(piece)).u8(static_cast<std::uint8_t>(jobSlot));
        writer.u8(static_cast<std::uint8_t>(blueprintSource)).u8(static_cast<std::uint8_t>(blueprintSlot));
        pet(writer, petSlot, petId);
        return writer.take();
    }

    core::Bytes stopMakingRequest(int jobSlot) { return {kManufacture, 3, static_cast<std::uint8_t>(jobSlot)}; } // sub 3: stop

    core::Bytes giveUpMakingRequest(int jobSlot) { return {kManufacture, 4, static_cast<std::uint8_t>(jobSlot)}; } // sub 4: give up

}
