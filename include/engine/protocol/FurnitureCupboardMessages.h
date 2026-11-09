#ifndef NORTH_ISLAND_PROTOCOL_FURNITURE_CUPBOARD_MESSAGES_H
#define NORTH_ISLAND_PROTOCOL_FURNITURE_CUPBOARD_MESSAGES_H

#include "core/Bytes.h"
#include "protocol/FurnitureCupboardItem.h"

#include <optional>

namespace protocol {

    /// @brief Decodes server 62/41: u16 piece, u8 place, u8 number, the 50-byte item record.
    /// @param payload Packet payload.
    /// @return The item, or nothing when malformed.
    std::optional<FurnitureCupboardItem> decodeFurnitureCupboardItem(const core::Bytes& payload);

    /// @brief Builds client 62/41: the bag stack goes into the cupboard whole.
    /// @param piece Furniture piece (u16).
    /// @param bagSlot Bag place (u8).
    /// @param place Place in the cupboard (u8).
    /// @return The encoded request.
    core::Bytes putInCupboardRequest(int piece, int bagSlot, int place);

    /// @brief Builds client 62/42: the item comes back to the bag.
    /// @param piece Furniture piece (u16).
    /// @param place Place in the cupboard (u8).
    /// @return The encoded request.
    core::Bytes takeFromCupboardRequest(int piece, int place);

}

#endif
