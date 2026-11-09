#include "protocol/PetRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kPets = 15; // action 15: pets
        constexpr std::uint8_t kActivePet = 19; // action 19: active (battle/following) pet

        /// @brief Narrows an int to a payload byte.
        /// @param value The value.
        /// @return The low byte.
        std::uint8_t byte(int value) { return static_cast<std::uint8_t>(value); }

    }

    core::Bytes petFightRequest(int npcId) {
        core::ByteWriter writer;
        writer.u8(kActivePet).u8(1).i32(npcId); // action 19, sub 1: pet fights
        return writer.take();
    }

    core::Bytes petRestRequest() { return {kActivePet, 2}; } // action 19, sub 2: pet rests

    core::Bytes renamePetRequest(int place, const std::string& name) {
        core::ByteWriter writer;
        writer.u8(kPets).u8(6).u8(byte(place)).text(name); // action 15, sub 6: rename
        return writer.take();
    }

    core::Bytes exilePetRequest(int place) { return {kPets, 2, byte(place)}; } // action 15, sub 2: send away

    core::Bytes learnSkillRequest(int skillId, int level) {
        core::ByteWriter writer;
        writer.u8(28).u8(1).u16(static_cast<std::uint16_t>(skillId)).u8(byte(level + 1)); // action 28, sub 1: learn a skill
        return writer.take();
    }

    core::Bytes ridePetRequest(int place, int npcId) {
        core::ByteWriter writer;
        writer.u8(kPets).u8(11).u8(byte(place)).i32(npcId); // action 15, sub 11: ride
        return writer.take();
    }

    core::Bytes dismountPetRequest(int place, int npcId) {
        core::ByteWriter writer;
        writer.u8(kPets).u8(12).u8(byte(place)).i32(npcId); // action 15, sub 12: get off
        return writer.take();
    }

}
