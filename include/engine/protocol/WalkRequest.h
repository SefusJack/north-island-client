#ifndef NORTH_ISLAND_PROTOCOL_WALK_REQUEST_H
#define NORTH_ISLAND_PROTOCOL_WALK_REQUEST_H

#include "core/Bytes.h"
#include "core/Point.h"

#include <cstdint>

namespace protocol {

    /// @brief Client 6/1: the own player walks to a map point; 6/2 with the same fields says the player stopped there (the server said so).
    /// @details Layout: 06 01, u8 direction (towards the destination from the last corner), u16 x, u16 y, u8 check (player id mod 13 + level), u8 random (0..219).
    struct WalkRequest {
        bool stopped = false;
        int direction = 0;
        core::Point destination;
        std::uint8_t check = 0;
        std::uint8_t random = 0;

        /// @brief Encodes the walk (6/1) or stop (6/2) message.
        /// @return The encoded request payload.
        core::Bytes encode() const;
    };

}

#endif
