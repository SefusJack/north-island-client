#ifndef NORTH_ISLAND_PROTOCOL_MALL_PURCHASE_LINE_H
#define NORTH_ISLAND_PROTOCOL_MALL_PURCHASE_LINE_H

namespace protocol {

    /// @brief One line of a mall purchase (75/1): the item, its category, how many of the row, its key.
    struct MallPurchaseLine {
        int itemId = 0;
        int category = 0;
        int count = 0;
        int key = 0;
    };

}

#endif
