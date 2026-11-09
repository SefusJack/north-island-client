#ifndef NORTH_ISLAND_PROTOCOL_STALL_LISTS_H
#define NORTH_ISLAND_PROTOCOL_STALL_LISTS_H

#include "core/Bytes.h"
#include "protocol/OwnStall.h"
#include "protocol/PetStallGoods.h"
#include "protocol/StallGoods.h"
#include "protocol/StallSale.h"
#include "protocol/StallSignRecord.h"

#include <optional>
#include <vector>

namespace protocol {

    /// @brief Decodes server 23/30, the own stall: pstr8 name, u8 sign, then u8 bag slot and i32 price records.
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The stall, or nullopt when the header is truncated.
    std::optional<OwnStall> decodeOwnStall(const core::Bytes& payload);

    /// @brief Decodes server 23/31, the stalls on the map: records of i32 player, pstr8 name, u8 sign.
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The complete records read before any truncation.
    std::vector<StallSignRecord> decodeStallSigns(const core::Bytes& payload);

    /// @brief Decodes server 23/33, the goods of an item stall.
    /// @details Layout: i32 owner, u32 card days, u32 card points, i32 card price, u8 card kind, then 37-byte items:
    /// u8 slot, u16 id, u8 count, u8 wear, i32 price, u8 a, u8 b, u16 crystal, f64 expiry, u8 fusion, u8 adjust,
    /// u16 look, u32 key, u16 box, i32 used, u16 extra.
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The goods, or nullopt when the header is truncated.
    std::optional<StallGoods> decodeStallGoods(const core::Bytes& payload);

    /// @brief Decodes server 23/34, an item sold from the own stall.
    /// @details Layout: u8 kind, u16 item, u8 count, i32 gold, and for kind 1 the u8 bag slot.
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The sale, or nullopt when truncated.
    std::optional<StallSale> decodeStallSale(const core::Bytes& payload);

    /// @brief Decodes server 23/113, the goods of a pet stall.
    /// @details Layout: i32 owner, then 218-byte pets: u8 place, u16 npc, i32 price, u8 level, i32 HP, i32 max HP,
    /// u16 max SP, 6 x 19-byte gear (u16 id, u8 wear, a, b, u16 crystal, f64 expiry, u8 fusion, adjust, u16 look),
    /// the 86-byte info block (u16 npc at 0, u8 amity at 47).
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The goods, or nullopt when the header is truncated.
    std::optional<PetStallGoods> decodePetStallGoods(const core::Bytes& payload);

    /// @brief Decodes server 23/114, a pet sold from the own stall.
    /// @details Layout: u8 kind, u16 npc, u8 count, i32 gold (no bag place).
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The sale, or nullopt when truncated.
    std::optional<StallSale> decodePetStallSale(const core::Bytes& payload);

}

#endif
