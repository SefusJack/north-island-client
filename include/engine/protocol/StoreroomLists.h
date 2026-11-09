#ifndef NORTH_ISLAND_PROTOCOL_STOREROOM_LISTS_H
#define NORTH_ISLAND_PROTOCOL_STOREROOM_LISTS_H

#include "core/Bytes.h"
#include "protocol/BagItemRecord.h"

#include <optional>
#include <vector>

namespace protocol {

    /// @brief The Storeroom contents: item records (the cell is the slot) and the piece of the Storeroom (0: none).
    /// @details Original: S->C 62/7 (RecvGarageItemList): n x the 29-byte bag record of 23/5, then u16 piece.
    struct StoreroomList {
        std::vector<BagItemRecord> records;
        int piece = 0;
    };

    /// @brief Decodes S->C 62/7, the whole Storeroom list.
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The list, or nullopt when the payload is too short or truncated.
    std::optional<StoreroomList> decodeStoreroomList(const core::Bytes& payload);

    /// @brief An item coming into a Storeroom cell, with the flag to show the obtain message.
    /// @details Original: S->C 62/11 (AddGarageItem) and 62/10 (RecvGarageItemAdd): u16 item, u8 count, u8 wear, u8 a, u8 b, i32 used, u16 extra,
    /// [62/11 only: u8 cell,] u16 insert, f64 expiry, u8 fusion, u8 adjust, u16 look, u16 box, u8 show message (1: "Obtain N Quantity <name>").
    /// The item's count is the amount that comes into the cell (slot 0 for 62/10: the first place it fits).
    struct StoreroomAdd {
        BagItemRecord record;
        bool showMessage = false;
    };

    /// @brief Decodes S->C 62/11 or 62/10, an item added to the Storeroom.
    /// @param payload Packet payload including the action and sub id bytes.
    /// @param namedCell True for 62/11, which carries the target cell; false for 62/10.
    /// @return The addition, or nullopt when truncated.
    std::optional<StoreroomAdd> decodeStoreroomAdd(const core::Bytes& payload, bool namedCell);

    /// @brief Items removed from the Storeroom, named by cell or by item id.
    /// @details Original: S->C 62/13 (RemoveGarageItem): u8 cell, u8 count; 62/12 (RemoveGarageItemById): u16 item, u8 count.
    struct StoreroomRemove {
        int cell = 0;
        int itemId = 0;
        int count = 0;
    };

    /// @brief Decodes S->C 62/13, removal by cell (u8 cell, u8 count).
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The removal, or nullopt when truncated.
    std::optional<StoreroomRemove> decodeStoreroomRemoveCell(const core::Bytes& payload);

    /// @brief Decodes S->C 62/12, removal by item id (u16 item, u8 count).
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The removal, or nullopt when truncated.
    std::optional<StoreroomRemove> decodeStoreroomRemoveItem(const core::Bytes& payload);

}

#endif
