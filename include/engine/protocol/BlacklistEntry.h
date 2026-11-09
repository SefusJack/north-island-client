#ifndef NORTH_ISLAND_PROTOCOL_BLACKLIST_ENTRY_H
#define NORTH_ISLAND_PROTOCOL_BLACKLIST_ENTRY_H

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief A player on the own black list (server 14/13).
    /// @details Original: RecvBlacklist.
    struct BlacklistEntry {
        std::int32_t playerId = 0;
        std::string name;
    };

}

#endif
