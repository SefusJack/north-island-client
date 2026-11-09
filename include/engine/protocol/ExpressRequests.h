#ifndef NORTH_ISLAND_PROTOCOL_EXPRESS_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_EXPRESS_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace protocol {

    /// @brief Builds client 25/20: the pet goes to the player with the items.
    /// @param petId Pet id (i32).
    /// @param targetId Receiving player id (i32).
    /// @param items Pairs of bag place (u8) and count (u8) for each item to send.
    /// @return The encoded request.
    core::Bytes expressStartRequest(std::int32_t petId, std::int32_t targetId, const std::vector<std::pair<int, int>>& items);

    /// @brief Builds client 25/21: the pet is called back.
    /// @return The encoded request.
    core::Bytes expressStopRequest();

}

#endif
