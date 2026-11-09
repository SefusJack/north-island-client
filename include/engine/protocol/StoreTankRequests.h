#ifndef NORTH_ISLAND_PROTOCOL_STORE_TANK_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_STORE_TANK_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds request 23/134 option 1: put fuel in the Store Tank.
    /// @details Layout: u8 bag place of the Tank, u8 bag place of the fuel, u8 row.
    /// @param tankSlot Bag place of the Tank.
    /// @param fuelSlot Bag place of the fuel.
    /// @param row Tank row.
    /// @return The encoded request payload.
    core::Bytes putInStoreTankRequest(int tankSlot, int fuelSlot, int row);

    /// @brief Builds request 23/134 option 2: take fuel out of the Store Tank into the bag.
    /// @details Layout: u8 bag place of the Tank, u8 row, u8 how many.
    /// @param tankSlot Bag place of the Tank.
    /// @param row Tank row.
    /// @param count How many to take.
    /// @return The encoded request payload.
    core::Bytes takeFromStoreTankRequest(int tankSlot, int row, int count);

    /// @brief Builds request 23/134 option 3: pour the fuel of the vehicle ridden.
    /// @details Layout: u8 bag place of the Tank, u8 row.
    /// @param tankSlot Bag place of the Tank.
    /// @param row Tank row.
    /// @return The encoded request payload.
    core::Bytes useStoreTankRequest(int tankSlot, int row);

}

#endif
