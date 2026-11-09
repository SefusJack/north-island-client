#ifndef NORTH_ISLAND_PROTOCOL_ACTIVITY_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_ACTIVITY_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    /// @brief Builds client 89/0: asks for the schedule, which the server may leave be when the version is its own.
    /// @param version Schedule version (i32) the client holds.
    /// @return The encoded request.
    core::Bytes activityListRequest(std::int32_t version);

    /// @brief Builds client 89/1: teleport to the event of that number in the list.
    /// @param number Event number (u8).
    /// @return The encoded request.
    core::Bytes activityTeleportRequest(int number);

}

#endif
