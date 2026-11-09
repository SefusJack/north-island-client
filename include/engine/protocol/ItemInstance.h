#ifndef NORTH_ISLAND_PROTOCOL_ITEM_INSTANCE_H
#define NORTH_ISLAND_PROTOCOL_ITEM_INSTANCE_H

#include <cstdint>

namespace protocol {

    /// @brief One item the player has (TThing / TEquip): which item, how many, and the server-kept state of this piece.
    /// @details An item id of 0 is an empty place.
    struct ItemInstance {
        int itemId = 0;
        int count = 0;
        int wear = 0;
        int byteA = 0;
        int byteB = 0;
        std::int32_t usedAmount = 0;
        int extraAmount = 0;
        int insertItemId = 0;
        double expiry = 0.0;
        int fusionLevel = 0;
        int attributeAdjust = 0;
        int lookItemId = 0;
        int boxItemId = 0;

        /// @brief Tells whether this place holds no item.
        /// @return True when the item id is 0.
        bool empty() const { return itemId == 0; }
    };

}

#endif
