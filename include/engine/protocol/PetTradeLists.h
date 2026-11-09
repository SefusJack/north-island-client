#ifndef NORTH_ISLAND_PROTOCOL_PET_TRADE_LISTS_H
#define NORTH_ISLAND_PROTOCOL_PET_TRADE_LISTS_H

#include "core/Bytes.h"
#include "protocol/PetTradeOffer.h"

#include <optional>

namespace protocol {

    /// @brief Decodes what the other side of a pet trade put up (server 25/12).
    /// @details Layout: i32 gold, u16 npc, u8 level, i32 max HP, i32 HP, u16 max SP, u16 SP, pstr8 name, u8 amity, u8 times upgraded,
    /// u8 n + n x 19-byte gear (u16 id, u8 wear, a, b, u16 crystal, f64 expiry, u8 fusion, adjust, u16 look), then the pet's 86-byte record.
    /// @param payload The packet payload.
    /// @return The offer, or nothing when malformed.
    std::optional<PetTradeOffer> decodePetTradeOffer(const core::Bytes& payload);

}

#endif
