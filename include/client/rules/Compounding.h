#ifndef NORTH_ISLAND_CLIENT_RULES_COMPOUNDING_H
#define NORTH_ISLAND_CLIENT_RULES_COMPOUNDING_H

#include "formats/ItemTable.h"

namespace client {

    /// @brief Tells why an item cannot go into the compound window, or 0 when it can.
    /// @details Original: CheckItemCombine, AC_Item.pas. Refusals: 1 marked so, 2 larger than a cell, 3 a vehicle, 4 no alchemy class
    /// (and not a job manteau, subtype 142), 6 item 61062, 255 no such item. Refusal 5 (the items that may not change hands) needs a list the game keeps elsewhere.
    /// @param items The item table.
    /// @param itemId The item id.
    /// @return 0 when it can be compounded, else the refusal code.
    int compoundRefusal(const formats::ItemTable& items, int itemId);

}

#endif
