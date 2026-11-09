#ifndef NORTH_ISLAND_PROTOCOL_REINFORCE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_REINFORCE_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 23/121: raise the piece's fusion level with a scroll.
    /// @param scrollSlot Bag place of the scroll (u8).
    /// @param equipSlot Bag place of the piece of gear (u8).
    /// @return The encoded payload.
    core::Bytes reinforceRequest(int scrollSlot, int equipSlot);

}

#endif
