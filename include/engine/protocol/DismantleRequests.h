#ifndef NORTH_ISLAND_PROTOCOL_DISMANTLE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_DISMANTLE_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds client 23/86: take the stone out of a piece with Tear Pliers.
    /// @param pieceSlot Bag place (u8) of the piece.
    /// @param pliersSlot Bag place (u8) of the pliers.
    /// @param mode 1 the Tear Plier, 2 the Small Tear Plier.
    /// @return The encoded request.
    core::Bytes dismantleRequest(int pieceSlot, int pliersSlot, int mode);

}

#endif
