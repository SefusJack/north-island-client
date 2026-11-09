#ifndef NORTH_ISLAND_PROTOCOL_TRADE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_TRADE_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>
#include <vector>

namespace protocol {

    /// @brief Builds request 25/1: ask a player to trade items (i32 player).
    /// @param playerId Player id asked.
    /// @return The encoded request payload.
    core::Bytes itemTradeRequest(std::int32_t playerId);

    /// @brief Builds request 25/2: put the offer up (i32 gold, then the bag slots offered).
    /// @param gold Gold offered.
    /// @param bagSlots Bag slots of the items offered.
    /// @return The encoded request payload.
    core::Bytes tradeOfferRequest(std::int32_t gold, const std::vector<int>& bagSlots);

    /// @brief Builds request 25/3: answer the item trade.
    /// @param answer 1 trade, 2 cancel.
    /// @return The encoded request payload.
    core::Bytes tradeAnswerRequest(int answer);

    /// @brief Builds request 25/10: ask a player to trade pets (i32 player).
    /// @param playerId Player id asked.
    /// @return The encoded request payload.
    core::Bytes petTradeRequest(std::int32_t playerId);

    /// @brief Builds request 25/11: put the pet offer up (i32 gold, u8 pet place, 0 for none).
    /// @param gold Gold offered.
    /// @param petPlace Pet place offered, 0 for none.
    /// @return The encoded request payload.
    core::Bytes petTradeOfferRequest(std::int32_t gold, int petPlace);

    /// @brief Builds request 25/12: answer the pet trade.
    /// @param answer 1 trade, 2 cancel.
    /// @return The encoded request payload.
    core::Bytes petTradeAnswerRequest(int answer);

}

#endif
