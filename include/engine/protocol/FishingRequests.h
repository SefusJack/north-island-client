#ifndef NORTH_ISLAND_PROTOCOL_FISHING_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_FISHING_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds client 23/53: fishing starts.
    /// @param rodSlot Bag place (u8) of the rod.
    /// @param rodKind The rod's kind (u8, the item rank).
    /// @return The encoded request.
    core::Bytes startFishingRequest(int rodSlot, int rodKind);

    /// @brief Builds client 23/54: fishing stops.
    /// @return The encoded request.
    core::Bytes stopFishingRequest();

}

#endif
