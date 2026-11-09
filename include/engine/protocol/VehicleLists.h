#ifndef NORTH_ISLAND_PROTOCOL_VEHICLE_LISTS_H
#define NORTH_ISLAND_PROTOCOL_VEHICLE_LISTS_H

#include "core/Bytes.h"
#include "protocol/ParkedVehicle.h"
#include "protocol/SceneRider.h"
#include "protocol/SceneVehicle.h"

#include <optional>
#include <vector>

namespace protocol {

    /// @brief Decodes server 23/102, the vehicles standing on the map: records of 16 bytes (u8 slot, i32 owner, u16 item, i32 x, i32 y, u8 bag place).
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The complete records in the payload.
    std::vector<SceneVehicle> decodeSceneVehicles(const core::Bytes& payload);

    /// @brief Decodes server 23/115, the riders on the map: records of 6 bytes (i32 player, u16 item).
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The complete records in the payload.
    std::vector<SceneRider> decodeSceneRiders(const core::Bytes& payload);

    /// @brief Decodes server 23/153, the parking lot: four records of 11 bytes (i32 item, u8 cell, i32 wear, u16 fuel).
    /// @details The cell of a record is its place in the list, counted from 1.
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The parked vehicles read from the cells.
    std::vector<ParkedVehicle> decodeParkList(const core::Bytes& payload);

    /// @brief Decodes server 23/154, one parking record, which names its cell.
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The record, or nullopt when the payload is too short.
    std::optional<ParkedVehicle> decodeParkSlot(const core::Bytes& payload);

}

#endif
