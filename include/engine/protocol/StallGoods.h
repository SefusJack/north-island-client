#ifndef NORTH_ISLAND_PROTOCOL_STALL_GOODS_H
#define NORTH_ISLAND_PROTOCOL_STALL_GOODS_H

#include "protocol/ItemInstance.h"

#include <cstdint>
#include <vector>

namespace protocol {

    /// @brief What someone's item stall sells: owner, recharge card and priced items.
    /// @details Original: server 23/33, RecvStallItemList. The card is days or points with a price and kind; no card means price 0.
    struct StallGoods {
        /// @brief One priced item offered by the stall.
        struct Item {
            int stallSlot = 0;
            ItemInstance item;
            std::int32_t price = 0;
            std::uint32_t echoKey = 0;
        };

        std::int32_t ownerId = 0;
        int cardDays = 0;
        int cardPoints = 0;
        std::int32_t cardPrice = 0;
        int cardKind = 0;
        std::vector<Item> items;
    };

}

#endif
