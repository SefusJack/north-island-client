#ifndef NORTH_ISLAND_PROTOCOL_CONFUSE_STONE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_CONFUSE_STONE_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds client 23/127: raise the stone mounted in a piece of gear by a step with a Chaos Crystal.
    /// @param equipSlot Bag place (u8) of the piece of gear.
    /// @param stoneSlot Bag place (u8) of the crystal.
    /// @return The encoded request.
    core::Bytes confuseStoneRequest(int equipSlot, int stoneSlot);

}

#endif
