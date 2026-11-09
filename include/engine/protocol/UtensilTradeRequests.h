#ifndef NORTH_ISLAND_PROTOCOL_UTENSIL_TRADE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_UTENSIL_TRADE_REQUESTS_H

#include "core/Bytes.h"
#include "protocol/TradeOffer.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace protocol {

    /// @brief Builds request 25/40: ask a player to trade large items (i32 player).
    /// @details Original: RequestVenderTrade.
    /// @param playerId Player id asked.
    /// @return The encoded request payload.
    core::Bytes utensilTradeRequest(std::int32_t playerId);

    /// @brief Builds request 25/41: offer large items (i32 gold, then the bag places offered).
    /// @details Original: ConfirmUtensilOffer builds the whole message.
    /// @param gold Gold offered.
    /// @param bagSlots Bag places of the items offered.
    /// @return The encoded request payload.
    core::Bytes utensilOfferRequest(std::int32_t gold, const std::vector<int>& bagSlots);

    /// @brief Builds request 25/42: answer the large-item trade.
    /// @param answer 1 trade, 2 cancel.
    /// @return The encoded request payload.
    core::Bytes utensilAnswerRequest(int answer);

    /// @brief Decodes server 25/41, the partner's large-item offer.
    /// @details Original: RecvUtensilTrade 41. Layout: i32 gold, then 9-byte items (u16 item, i32 used amount, u16 extra amount, u8 wear).
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The offer, or nullopt when the gold is truncated.
    std::optional<TradeOffer> decodeUtensilOffer(const core::Bytes& payload);

}

#endif
