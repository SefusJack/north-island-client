#ifndef NORTH_ISLAND_PROTOCOL_FURNITURE_STALL_ITEM_H
#define NORTH_ISLAND_PROTOCOL_FURNITURE_STALL_ITEM_H

#include "core/Bytes.h"
#include "protocol/ItemInstance.h"

#include <cstdint>
#include <optional>

namespace protocol {

    /// @brief An item put on a shelf or in a vending machine of a house (server 62/21): seller, stall number, piece and position, the owner's bag place, count, price and the 50-byte item record.
    /// @details Original: HandleUtensilPacket. The record's word at 1 and its integer at 6 are sent back when someone buys.
    struct FurnitureStallItem {
        std::int32_t ownerId = 0;
        int stallSlot = 0;
        int furnIndex = 0;
        int posInFurn = 0;
        int gridCell = 0;
        int count = 0;
        std::int32_t price = 0;
        ItemInstance item;
        int echoItem = 0;
        std::int32_t echoPrice = 0;
    };

    /// @brief Decodes the whole payload of server 62/21.
    /// @param payload Packet payload.
    /// @return The stall item, or nothing when malformed.
    std::optional<FurnitureStallItem> decodeFurnitureStallItem(const core::Bytes& payload);

}

#endif
