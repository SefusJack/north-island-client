#ifndef NORTH_ISLAND_PROTOCOL_TRADE_LISTS_H
#define NORTH_ISLAND_PROTOCOL_TRADE_LISTS_H

#include "core/Bytes.h"
#include "protocol/TradeOffer.h"

#include <optional>

namespace protocol {

    /// @brief Decodes server 25/3, the partner's item trade offer.
    /// @details Layout: i32 gold, then 28-byte items: u16 id, u8 count, u8 wear, i32 used, u16 extra, u8 a, u8 b, u16 crystal, f64 expiry, u8 fusion, u8 adjust, u16 look, u16 box.
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The offer, or nullopt when the gold is truncated.
    std::optional<TradeOffer> decodeTradeOffer(const core::Bytes& payload);

}

#endif
