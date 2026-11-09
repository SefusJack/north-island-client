#ifndef NORTH_ISLAND_PROTOCOL_STALL_SALE_H
#define NORTH_ISLAND_PROTOCOL_STALL_SALE_H

#include <cstdint>

namespace protocol {

    /// @brief Something sold from the own stall and the gold it brought.
    /// @details Original: server 23/34 RecvStallSale, 23/114 RecvPetStallSale. Kind 1 is items (from a bag place) or a pet (itemId then holds its kind), 2 is a recharge card.
    struct StallSale {
        int kind = 0;
        int itemId = 0;
        int count = 0;
        std::int32_t money = 0;
        int bagSlot = 0;
    };

}

#endif
