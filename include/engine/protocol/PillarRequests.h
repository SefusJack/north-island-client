#ifndef NORTH_ISLAND_PROTOCOL_PILLAR_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_PILLAR_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 65/11: the pillar changes of a house floor (SendPillarChanges).
    /// @details Layout: u16 the Handcart's piece, u8 mode (1 all 48 bytes of the floor, 2 pairs of a byte's number and its bits), the data.
    /// @param body What HousePillars::changeBody gives.
    /// @return The encoded payload.
    core::Bytes pillarChangesRequest(const core::Bytes& body);

}

#endif
