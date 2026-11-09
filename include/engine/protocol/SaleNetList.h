#ifndef NORTH_ISLAND_PROTOCOL_SALE_NET_LIST_H
#define NORTH_ISLAND_PROTOCOL_SALE_NET_LIST_H

#include "protocol/ItemInstance.h"

#include <cstdint>
#include <vector>

namespace protocol {

    /// @brief The sale net's list: its value and 8-byte records (place 1..10, item, state 2 on sale / 3 competed for / else free).
    /// @details Original: server 23/63 (RecvSaleNetList): i32 value, then u8 place, u16 item, u8 count, u8 wear, u8, u8, u8 state.
    struct SaleNetList {
        /// @brief One place of the list.
        struct Entry {
            int place = 0;
            ItemInstance item;
            int state = 0;
        };

        std::int32_t value = 0;
        std::vector<Entry> entries;
    };

}

#endif
