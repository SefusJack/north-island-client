#include "protocol/VehicleRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kPets = 15; // action 15: pets (vehicle requests such as 15/9 are sent here)

        /// @brief Builds a 15 request of the shape sub id, u8 bag place, u16 item.
        /// @param sub Sub id.
        /// @param bagSlot Bag place of the vehicle.
        /// @param itemId Item id of the vehicle.
        /// @return The encoded request payload.
        core::Bytes withItem(std::uint8_t sub, int bagSlot, int itemId) {
            core::ByteWriter writer;
            writer.u8(kPets).u8(sub).u8(static_cast<std::uint8_t>(bagSlot)).u16(static_cast<std::uint16_t>(itemId));
            return writer.take();
        }

    }

    core::Bytes placeVehicleRequest(int bagSlot, int itemId) { return withItem(9, bagSlot, itemId); } // 9 = place vehicle sub id

    core::Bytes boardVehicleRequest(int bagSlot, int itemId) { return withItem(7, bagSlot, itemId); } // 7 = board vehicle sub id

    core::Bytes parkVehicleRequest(int bagSlot, int itemId) { return withItem(8, bagSlot, itemId); } // 8 = park ridden vehicle sub id

    core::Bytes useVehicleRequest(int bagSlot, int itemId) { return withItem(10, bagSlot, itemId); } // 10 = use placed vehicle sub id

    core::Bytes enterWaterVehicleRequest(int bagSlot, int itemId) { return withItem(14, bagSlot, itemId); } // 14 = enter water with vehicle sub id

    core::Bytes repairVehicleRequest(int bagSlot) { return {23, 95, static_cast<std::uint8_t>(bagSlot)}; } // 23 = item action, 95 = repair vehicle sub id

    core::Bytes capsuleRequest(int capsuleSlot, int vehicleSlot) {
        // 23 = item action, 125 = Space Capsule sub id
        return {23, 125, static_cast<std::uint8_t>(capsuleSlot), static_cast<std::uint8_t>(vehicleSlot)};
    }

    core::Bytes parkInLotRequest(int bagSlot, int cell) {
        // 23 = item action, 92 = park in lot sub id, then the form's own u16 word 0
        return {23, 92, 0, 0, static_cast<std::uint8_t>(bagSlot), static_cast<std::uint8_t>(cell)};
    }

    core::Bytes redeemFromLotRequest(int cell) {
        // 23 = item action, 93 = redeem from lot sub id; 0xFF and >> 8 split the u16 cell into low and high byte
        return {23, 93, static_cast<std::uint8_t>(cell & 0xFF), static_cast<std::uint8_t>(cell >> 8)};
    }

    core::Bytes transmitRequest(int place) { return {69, static_cast<std::uint8_t>(place)}; } // 69 = vehicle transmit action

    core::Bytes wreckSeenRequest(std::int32_t ownerId) {
        core::ByteWriter writer;
        writer.u8(kPets).u8(13).i32(ownerId); // 13 = wreck seen sub id
        return writer.take();
    }

}
