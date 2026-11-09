#ifndef NORTH_ISLAND_PROTOCOL_MAGIC_BOX_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_MAGIC_BOX_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 23/105 (u8 2): the piece goes back to its own look in the Rebirth Box.
    /// @param boxSlot Bag place of the Rebirth Box (u8).
    /// @param pieceSlot Bag place of the piece (u8).
    /// @return The encoded payload.
    core::Bytes magicBoxRequest(int boxSlot, int pieceSlot);

}

#endif
