#ifndef NORTH_ISLAND_PROTOCOL_TRADE_OFFER_H
#define NORTH_ISLAND_PROTOCOL_TRADE_OFFER_H

#include "protocol/ItemInstance.h"

#include <cstdint>
#include <vector>

namespace protocol {

    /// @brief What the other side of an item trade put up: gold and items.
    /// @details Original: server 25/3, RecvPartnerTradeItems.
    struct TradeOffer {
        std::int32_t gold = 0;
        std::vector<ItemInstance> items;
    };

}

#endif
