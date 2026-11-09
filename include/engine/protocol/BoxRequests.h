#ifndef NORTH_ISLAND_PROTOCOL_BOX_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_BOX_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds client 23/131 (Picnic Box) or 23/132 (EXP Bag) op 1: put a stack into a cell.
    /// @param picnic True for the Picnic Box, false for the EXP Bag.
    /// @param bagSlot Bag place (u8) of the stack.
    /// @param cell Cell (u8).
    /// @param boxSlot Bag place (u8) of the box.
    /// @return The encoded request.
    core::Bytes boxPutRequest(bool picnic, int bagSlot, int cell, int boxSlot);

    /// @brief Builds client 23/131 (Picnic Box) or 23/132 (EXP Bag) op 2: take the cell's stack back into the bag.
    /// @param picnic True for the Picnic Box, false for the EXP Bag.
    /// @param cell Cell (u8).
    /// @param boxSlot Bag place (u8) of the box.
    /// @return The encoded request.
    core::Bytes boxTakeRequest(bool picnic, int cell, int boxSlot);

    /// @brief Builds client 23/131 (Picnic Box) or 23/132 (EXP Bag) op 3: use one from a cell.
    /// @param picnic True for the Picnic Box, false for the EXP Bag.
    /// @param cell Cell (u8).
    /// @param target Pet order (the Picnic Box; 0 the player), or 1 for a use the form started by itself (the EXP Bag).
    /// @param boxSlot Bag place (u8) of the box.
    /// @return The encoded request.
    core::Bytes boxUseRequest(bool picnic, int cell, int target, int boxSlot);

}

#endif
