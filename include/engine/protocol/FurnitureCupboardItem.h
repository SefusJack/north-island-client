#ifndef NORTH_ISLAND_PROTOCOL_FURNITURE_CUPBOARD_ITEM_H
#define NORTH_ISLAND_PROTOCOL_FURNITURE_CUPBOARD_ITEM_H

#include "protocol/ItemInstance.h"

namespace protocol {

    /// @brief An item put in a place of a cupboard of a house (server 62/41): the piece, the place (1..10), the number it is kept under (1..50) and the item.
    /// @details Original: HandleUtensilPacket; the item is the 50-byte TCarItem_WS_Menu record. A car put in a garage cell (62/47) comes the same way (cell 1..4, number 1..4).
    struct FurnitureCupboardItem {
        int furnIndex = 0;
        int place = 0;
        int id = 0;
        ItemInstance item;
    };

}

#endif
