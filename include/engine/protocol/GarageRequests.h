#ifndef NORTH_ISLAND_PROTOCOL_GARAGE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_GARAGE_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds client 62/51: the Garage was clicked (the server answers 62/51 with its restrained state).
    /// @param piece Furniture piece (u16).
    /// @return The encoded request.
    core::Bytes openGarageRequest(int piece);

    /// @brief Builds client 62/47: park the vehicle of the bag in a cell.
    /// @param piece Furniture piece (u16).
    /// @param bagSlot Bag place (u8).
    /// @param cell Garage cell (u8).
    /// @return The encoded request.
    core::Bytes putInGarageRequest(int piece, int bagSlot, int cell);

    /// @brief Builds client 62/48: take the vehicle of a cell back into the bag.
    /// @param piece Furniture piece (u16).
    /// @param cell Garage cell (u8).
    /// @return The encoded request.
    core::Bytes takeFromGarageRequest(int piece, int cell);

}

#endif
