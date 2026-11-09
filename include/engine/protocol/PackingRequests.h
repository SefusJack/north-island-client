#ifndef NORTH_ISLAND_PROTOCOL_PACKING_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_PACKING_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 23/112 (u8 1): a piece is packed into an empty Present Case (TPackingForm).
    /// @param boxSlot Bag place of the empty case (u8).
    /// @param pieceSlot Bag place of the piece (u8).
    /// @return The encoded payload.
    core::Bytes packingRequest(int boxSlot, int pieceSlot);

    /// @brief Builds 23/112 (u8 2, last u8 0): the packed piece is taken out of the case.
    /// @param boxSlot Bag place of the packed case (u8).
    /// @return The encoded payload.
    core::Bytes openPackedBoxRequest(int boxSlot);

}

#endif
