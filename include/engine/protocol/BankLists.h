#ifndef NORTH_ISLAND_PROTOCOL_BANK_LISTS_H
#define NORTH_ISLAND_PROTOCOL_BANK_LISTS_H

#include "core/Bytes.h"
#include "protocol/BagItemRecord.h"
#include "protocol/ItemInstance.h"

#include <optional>
#include <vector>

namespace protocol {

    /// @brief Decodes server 30/1: u8 place and a record for each item.
    /// @details Original: RecvBankItemList.
    /// @param payload Packet payload.
    /// @return The bank items with their places.
    std::vector<BagItemRecord> decodeBankItems(const core::Bytes& payload);

    /// @brief Decodes server 30/2: a record; the client finds it a place.
    /// @details Original: RecvBankItemAdd.
    /// @param payload Packet payload.
    /// @return The item, or nothing when malformed.
    std::optional<ItemInstance> decodeBankItem(const core::Bytes& payload);

    /// @brief Decodes server 30/4: u8 place, then a record.
    /// @details Original: RecvBankItemPut.
    /// @param payload Packet payload.
    /// @return The item with its place, or nothing when malformed.
    std::optional<BagItemRecord> decodeBankItemPut(const core::Bytes& payload);

    /// @brief Decodes server 30/10: a record, then u8 place.
    /// @details Original: RecvBankItemAtSlot.
    /// @param payload Packet payload.
    /// @return The item with its place, or nothing when malformed.
    std::optional<BagItemRecord> decodeBankItemAtSlot(const core::Bytes& payload);

}

#endif
