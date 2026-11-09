#ifndef NORTH_ISLAND_PROTOCOL_POINT_STORE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_POINT_STORE_REQUESTS_H

#include "core/Bytes.h"

#include <utility>
#include <vector>

namespace protocol {

    /// @brief Builds 80/1: buy the lines of the cart.
    /// @param lines Pairs of (row, count); the row is 1-based (u8 each).
    /// @return The encoded payload.
    core::Bytes pointStoreBuyRequest(const std::vector<std::pair<int, int>>& lines);

    /// @brief Builds 80/2 (player) or 80/3 (fighting pet): buy the next level with game points.
    /// @details Original: SendBuyPlayerLevel, SendBuyPetLevel.
    /// @param pet True for the fighting pet.
    /// @return The encoded payload.
    core::Bytes pointStoreLevelRequest(bool pet);

}

#endif
