#ifndef NORTH_ISLAND_PROTOCOL_UTENSIL_STALL_GOODS_H
#define NORTH_ISLAND_PROTOCOL_UTENSIL_STALL_GOODS_H

#include "protocol/ItemInstance.h"

#include <cstdint>
#include <vector>

namespace protocol {

    /// @brief What someone's stall of large items sells: the owner and up to three pieces with their prices.
    /// @details Original: server 23/133, RecvUtensilStallList.
    struct UtensilStallGoods {
        /// @brief One priced large item; listPlace is the owner's bag place, sent back when buying.
        struct Piece {
            int listPlace = 0;
            ItemInstance item;
            std::int32_t price = 0;
        };

        std::int32_t ownerId = 0;
        std::vector<Piece> pieces;
    };

}

#endif
