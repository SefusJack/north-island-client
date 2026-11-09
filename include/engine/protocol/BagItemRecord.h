#ifndef NORTH_ISLAND_PROTOCOL_BAG_ITEM_RECORD_H
#define NORTH_ISLAND_PROTOCOL_BAG_ITEM_RECORD_H

#include "protocol/ItemInstance.h"

namespace protocol {

    /// @brief An item in a place of the bag (slot 1..50): one 29-byte record of server 23/5 and 23/8.
    struct BagItemRecord {
        int slot = 0;
        ItemInstance item;
    };

}

#endif
