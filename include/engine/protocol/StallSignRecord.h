#ifndef NORTH_ISLAND_PROTOCOL_STALL_SIGN_RECORD_H
#define NORTH_ISLAND_PROTOCOL_STALL_SIGN_RECORD_H

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief A stall someone keeps on the map: whose, its name and which stall picture it shows.
    /// @details Original: server 23/31, RecvItemStallList.
    struct StallSignRecord {
        std::int32_t playerId = 0;
        std::string name;
        int sign = 0;
    };

}

#endif
