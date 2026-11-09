#ifndef NORTH_ISLAND_PROTOCOL_ITEM_BAG_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_ITEM_BAG_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Direction of an Item Bag move (68/1 to the bag, 68/2 within the Item Bag, 68/3 to the Item Bag).
    /// @details Original: the Item Bag is the 5 x 5 second bag of the Space Contract (action 68); the server echoes an order word for word.
    enum class ItemBagMove { ToBag = 1, Within = 2, ToGrid = 3 };

    /// @brief Builds 68/kind: move items between the bag and the Item Bag.
    /// @param kind Move kind (the sub code).
    /// @param from Source cell or bag place (u8).
    /// @param to Destination cell or bag place (u8).
    /// @param count Item count (u8).
    /// @return The encoded payload.
    core::Bytes itemBagMoveRequest(ItemBagMove kind, int from, int to, int count);

}

#endif
