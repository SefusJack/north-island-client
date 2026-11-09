#ifndef NORTH_ISLAND_PROTOCOL_ITEM_LISTS_H
#define NORTH_ISLAND_PROTOCOL_ITEM_LISTS_H

#include "core/Bytes.h"
#include "protocol/BagItemRecord.h"
#include "protocol/ItemInstance.h"

#include <optional>
#include <vector>

namespace protocol {

    /// @brief Decodes the bag list.
    /// @details Original: server 23/5, 29-byte records (RecvBagItemList).
    /// @param payload The packet payload.
    /// @return The bag item records.
    std::vector<BagItemRecord> decodeBagItems(const core::Bytes& payload);

    /// @brief Decodes one item added to a place of the bag (server 23/8, the 23/5 record).
    /// @param payload The packet payload.
    /// @return The record, or nothing when malformed.
    std::optional<BagItemRecord> decodeBagItem(const core::Bytes& payload);

    /// @brief Decodes what the own player wears.
    /// @details Original: server 23/11, 19-byte records (RecvEquipList); the item's Item.dat equipment slot says where it goes.
    /// @param payload The packet payload.
    /// @return The worn items.
    std::vector<ItemInstance> decodeEquipment(const core::Bytes& payload);

}

#endif
