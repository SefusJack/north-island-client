#ifndef NORTH_ISLAND_PROTOCOL_POTENTIAL_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_POTENTIAL_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 23/126: use a Potential Pill (SendPotential).
    /// @param bagSlot Bag place (u8).
    /// @param petPlace Pet place (u8, 0: the player).
    /// @return The encoded payload.
    core::Bytes potentialRequest(int bagSlot, int petPlace);

    /// @brief Builds 23/130: use an Anti-Potential Pill on a pet (SendPotentialReturn).
    /// @param bagSlot Bag place (u8).
    /// @param petPlace Pet place (u8).
    /// @return The encoded payload.
    core::Bytes potentialReturnRequest(int bagSlot, int petPlace);

}

#endif
