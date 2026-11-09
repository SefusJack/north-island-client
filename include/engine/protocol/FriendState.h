#ifndef NORTH_ISLAND_PROTOCOL_FRIEND_STATE_H
#define NORTH_ISLAND_PROTOCOL_FRIEND_STATE_H

#include <cstdint>

namespace protocol {

    /// @brief A friend's online state (server 14/9): 0 online, 1 busy, 2 hidden, 3 offline.
    struct FriendState {
        std::int32_t id = 0;
        int state = 3;
    };

}

#endif
