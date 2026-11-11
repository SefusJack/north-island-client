#ifndef NORTH_ISLAND_PROTOCOL_MALL_MESSAGES_H
#define NORTH_ISLAND_PROTOCOL_MALL_MESSAGES_H

#include "core/Bytes.h"
#include "protocol/MallPurchaseLine.h"
#include "protocol/MallRow.h"

#include <vector>

namespace protocol {

    /// @brief Decodes the item mall's catalog (server 75/1).
    /// @details Layout: u16 n, then n 10-byte rows (u16 item, u8 count, u16 price, u8 discount, u8 hot, u8 category, u16 key).
    /// @param payload The packet payload.
    /// @return The catalog rows.
    std::vector<MallRow> decodeMallCatalog(const core::Bytes& payload);

    /// @brief Builds 75/1: the cart is bought.
    /// @details Original: SendCartPurchase; u8 n, then n x (u16 item, u8 category, u8 count, u16 key).
    /// @param lines The purchase lines.
    /// @return The encoded payload.
    core::Bytes mallBuyRequest(const std::vector<MallPurchaseLine>& lines);

    /// @brief Builds 75/2: the catalog is asked for again (SendOpenItemMall).
    /// @return The encoded payload.
    core::Bytes mallCatalogRequest();

    /// @brief Builds 75/3: the item in the mall's forge is to be forged (SendForge).
    /// @param bagSlot Bag place (u8).
    /// @return The encoded payload.
    core::Bytes mallForgeRequest(int bagSlot);

    /// @brief Builds 75/4: a slot machine of the games tab is asked for (RequestShopCategory).
    /// @param kind Kind (u8).
    /// @return The encoded payload.
    core::Bytes mallGameRequest(int kind);

    /// @brief Builds 34/1: ask how many IM points the account has (answered by 35/4).
    /// @param forPayment 1 when asked to pay the cart.
    /// @return The encoded payload.
    core::Bytes imPointsRequest(int forPayment);

}

#endif
