#ifndef NORTH_ISLAND_PROTOCOL_GIFT_CARD_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_GIFT_CARD_REQUESTS_H

#include "core/Bytes.h"

#include <vector>

namespace protocol {

    /// @brief Builds client 23/133/1: the card of a bag place was used.
    /// @param bagSlot Bag place (u8).
    /// @param cardId Card (u16).
    /// @param version Version (u8) of the card's list the client holds; 0 none.
    /// @return The encoded request.
    core::Bytes giftCardUseRequest(int bagSlot, int cardId, int version);

    /// @brief Builds client 23/133/2: exchange the card that many times for the choice.
    /// @param bagSlot Bag place (u8).
    /// @param times Number of exchanges (u8).
    /// @param choice Place in the card's list, from 1 (u8).
    /// @param itemId Item (u16).
    /// @return The encoded request.
    core::Bytes giftCardExchangeRequest(int bagSlot, int times, int choice, int itemId);

    /// @brief Builds client 90/1/1: ask for the lists of the cards the client lacks (u16 n, n x u16 card).
    /// @param cards Card ids.
    /// @return The encoded request.
    core::Bytes giftCardListRequest(const std::vector<int>& cards);

}

#endif
