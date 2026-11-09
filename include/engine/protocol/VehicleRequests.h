#ifndef NORTH_ISLAND_PROTOCOL_VEHICLE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_VEHICLE_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    /// @brief Builds request 15/9: put the vehicle from the bag onto the map (u8 bag place, u16 item).
    /// @param bagSlot Bag place of the vehicle.
    /// @param itemId Item id of the vehicle.
    /// @return The encoded request payload.
    core::Bytes placeVehicleRequest(int bagSlot, int itemId);

    /// @brief Builds request 15/7: get on the vehicle standing on the map (u8 bag place, u16 item).
    /// @param bagSlot Bag place of the vehicle.
    /// @param itemId Item id of the vehicle.
    /// @return The encoded request payload.
    core::Bytes boardVehicleRequest(int bagSlot, int itemId);

    /// @brief Builds request 15/8: leave the vehicle ridden where it is (u8 bag place, u16 item).
    /// @param bagSlot Bag place of the vehicle.
    /// @param itemId Item id of the vehicle.
    /// @return The encoded request payload.
    core::Bytes parkVehicleRequest(int bagSlot, int itemId);

    /// @brief Builds request 15/10: use the placed vehicle of the bag, to ride it or to take it back by its right click (u8 bag place, u16 item).
    /// @param bagSlot Bag place of the vehicle.
    /// @param itemId Item id of the vehicle.
    /// @return The encoded request payload.
    core::Bytes useVehicleRequest(int bagSlot, int itemId);

    /// @brief Builds request 15/14: put the vehicle in the water and ride it, when walking into water (u8 bag place, u16 item).
    /// @param bagSlot Bag place of the vehicle.
    /// @param itemId Item id of the vehicle.
    /// @return The encoded request payload.
    core::Bytes enterWaterVehicleRequest(int bagSlot, int itemId);

    /// @brief Builds request 23/95: repair the vehicle in the garage form (u8 bag place).
    /// @param bagSlot Bag place of the vehicle.
    /// @return The encoded request payload.
    core::Bytes repairVehicleRequest(int bagSlot);

    /// @brief Builds request 23/125: the Space Capsule form's OK (u8 capsule place, u8 vehicle place).
    /// @param capsuleSlot Bag place of the capsule.
    /// @param vehicleSlot Bag place of the vehicle.
    /// @return The encoded request payload.
    core::Bytes capsuleRequest(int capsuleSlot, int vehicleSlot);

    /// @brief Builds request 23/92: park the vehicle of the bag in a cell of the parking lot.
    /// @details Layout: u16 (the form's own word, 0), u8 bag place, u8 cell.
    /// @param bagSlot Bag place of the vehicle.
    /// @param cell Parking lot cell.
    /// @return The encoded request payload.
    core::Bytes parkInLotRequest(int bagSlot, int cell);

    /// @brief Builds request 23/93: take the vehicle of a parking lot cell back into the bag (u16 cell).
    /// @param cell Parking lot cell.
    /// @return The encoded request payload.
    core::Bytes redeemFromLotRequest(int cell);

    /// @brief Builds request 69/place: the trip of the vehicle to a place of the list starts.
    /// @param place Place of the list, counted from 1.
    /// @return The encoded request payload.
    core::Bytes transmitRequest(int place);

    /// @brief Builds request 15/13: the wreck of a vehicle was seen to the end (i32 owner).
    /// @param ownerId Player id of the vehicle owner.
    /// @return The encoded request payload.
    core::Bytes wreckSeenRequest(std::int32_t ownerId);

}

#endif
