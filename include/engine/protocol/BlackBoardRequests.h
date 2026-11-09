#ifndef NORTH_ISLAND_PROTOCOL_BLACK_BOARD_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_BLACK_BOARD_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief Builds client 72/sub for a house's blackboard: sub 1 (board clicked) no data, sub 2 (write) u8 row and the text, sub 3 (delete) u8 row.
    /// @param sub Sub id (1 click, 2 write, 3 delete).
    /// @param houseOwnerId Player id of the house owner (i32).
    /// @param data Sub-specific data bytes.
    /// @return The encoded request.
    core::Bytes blackBoardRequest(int sub, std::int32_t houseOwnerId, const std::string& data = {});

}

#endif
