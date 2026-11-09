#include "protocol/HouseRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kHouse = 62; // action 62: house / furniture
        constexpr std::uint8_t kItems = 23; // action 23: items

    }

    core::Bytes placeFurnitureRequest(
        int bagSlot,
        int cellX,
        int cellY,
        int cellZ,
        int facing
    ) {
        core::ByteWriter writer;
        writer.u8(kHouse)
            .u8(1)
            .u8(static_cast<std::uint8_t>(bagSlot))
            .i32(cellX)
            .i32(cellY)
            .i32(cellZ)
            .u8(static_cast<std::uint8_t>(facing)); // sub 1: place furniture
        return writer.take();
    }

    core::Bytes takeBackFurnitureRequest(int piece) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(2).u16(static_cast<std::uint16_t>(piece)); // sub 2: take furniture back
        return writer.take();
    }

    core::Bytes moveFurnitureRequest(
        int piece,
        int cellX,
        int cellY,
        int cellZ,
        int facing
    ) {
        core::ByteWriter writer;
        writer.u8(kHouse)
            .u8(3)
            .u16(static_cast<std::uint16_t>(piece))
            .i32(cellX)
            .i32(cellY)
            .i32(cellZ)
            .u8(static_cast<std::uint8_t>(facing)); // sub 3: move or turn furniture
        return writer.take();
    }

    core::Bytes toolOnPlatformRequest(int bagSlot, int platform) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(4).u8(static_cast<std::uint8_t>(bagSlot)).u16(static_cast<std::uint16_t>(platform)); // sub 4: tool onto the Work Platform
        return writer.take();
    }

    core::Bytes recycleItemRequest(int bagSlot, int count, int bin) {
        core::ByteWriter writer;
        writer.u8(kHouse)
            .u8(5)
            .u8(static_cast<std::uint8_t>(bagSlot))
            .u8(static_cast<std::uint8_t>(count))
            .u16(static_cast<std::uint16_t>(bin)); // sub 5: items into a recycling bin
        return writer.take();
    }

    core::Bytes recycleFurnitureRequest(int piece, int bin) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(6).u16(static_cast<std::uint16_t>(piece)).u16(static_cast<std::uint16_t>(bin)); // sub 6: placed piece into a recycling bin
        return writer.take();
    }

    core::Bytes useFloorItemRequest(int bagSlot) { return {kHouse, 10, static_cast<std::uint8_t>(bagSlot)}; } // sub 10: floor item for the room

    core::Bytes useWallItemRequest(int bagSlot) { return {kHouse, 11, static_cast<std::uint8_t>(bagSlot)}; } // sub 11: wall item for the room

    core::Bytes pianoKeyRequest(int note) { return {kHouse, 57, static_cast<std::uint8_t>(note)}; } // sub 57: Piano key struck

    core::Bytes pianoSitRequest(int piece) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(99).u16(static_cast<std::uint16_t>(piece)); // sub 99 of action 23: sat down at the Piano
        return writer.take();
    }

    core::Bytes stopUsingUtensilRequest() { return {kItems, 72}; } // sub 72 of action 23: stopped using the piece

    core::Bytes sitUtensilRequest(int piece, int seat) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(71).u16(static_cast<std::uint16_t>(piece)).u8(static_cast<std::uint8_t>(seat)); // sub 71 of action 23: sat down at a seat
        return writer.take();
    }

    core::Bytes newYearCrackerRequest(int piece) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(64).u16(static_cast<std::uint16_t>(piece)); // sub 64: New Year Cracker lit
        return writer.take();
    }

    core::Bytes guguClockRequest(int piece) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(65).u16(static_cast<std::uint16_t>(piece)); // sub 65: Gugu Clock set going
        return writer.take();
    }

    core::Bytes stairsRequest(int piece) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(14).u16(static_cast<std::uint16_t>(piece)); // sub 14: Elevator or Stairs clicked
        return writer.take();
    }

    core::Bytes batheRequest(std::int32_t ownerId, int piece) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(55).i32(ownerId).u16(static_cast<std::uint16_t>(piece)); // sub 55 of action 23: steps into a tub
        return writer.take();
    }

    core::Bytes stopBatheRequest(std::int32_t ownerId, int piece) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(56).i32(ownerId).u16(static_cast<std::uint16_t>(piece)); // sub 56 of action 23: gets out of a tub
        return writer.take();
    }

    core::Bytes openFishTankRequest() { return {kHouse, 56}; } // sub 56: open the fish tank

    core::Bytes fishTankRequest(int request) { return {kItems, 97, static_cast<std::uint8_t>(request)}; } // sub 97 of action 23: fish tank request

    core::Bytes fishTankKindRequest(int request, int kind) {
        return {kItems, 98, static_cast<std::uint8_t>(request), static_cast<std::uint8_t>(kind)};
    } // sub 98 of action 23: fish tank request with a kind

    core::Bytes houseBoxRequest(int option, int piece, int place, std::optional<int> bagSlot) {
        core::ByteWriter writer;
        writer.u8(kHouse)
            .u8(52)
            .u8(static_cast<std::uint8_t>(option))
            .u8(static_cast<std::uint8_t>(piece))
            .u8(static_cast<std::uint8_t>(place)); // sub 52: storage piece put/take
        if (bagSlot) {
            writer.u8(static_cast<std::uint8_t>(*bagSlot));
        }
        return writer.take();
    }

    core::Bytes houseMusicRequest(int song, const std::string& name) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(61).i32(song).text(name); // sub 61: Music Center song
        return writer.take();
    }

    core::Bytes gasDepositRequest(bool small, const std::vector<int>& bagSlots) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(small ? 63 : 62); // sub 63: Gas Storage, 62: Big Gas Storage
        for (const int slot : bagSlots) {
            writer.u8(static_cast<std::uint8_t>(slot));
        }
        return writer.take();
    }

    core::Bytes openPortraitRequest(int piece) { return {kHouse, 66, static_cast<std::uint8_t>(piece)}; } // sub 66: Picture Frame clicked

}
