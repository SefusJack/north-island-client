#ifndef NORTH_ISLAND_PROTOCOL_GROUND_ITEM_RECORD_H
#define NORTH_ISLAND_PROTOCOL_GROUND_ITEM_RECORD_H

#include "core/Point.h"

#include <cstdint>

namespace protocol {

    /// @brief Something lying on the map (server 23/4, 15 bytes a record).
    /// @details Layout: u8 kind (2 gold, else an item), u16 index (1..255), i32 the item or the gold, u16 x, u16 y, i32 extra (added to the y it is sorted by).
    struct GroundItemRecord {
        int kind = 1;
        int index = 0;
        std::int32_t itemOrGold = 0;
        core::Point position;
        std::int32_t extra = 0;
    };

}

#endif
