#ifndef NORTH_ISLAND_CLIENT_MODELS_SOCIAL_FRIEND_H
#define NORTH_ISLAND_CLIENT_MODELS_SOCIAL_FRIEND_H

#include "protocol/FriendRecord.h"

namespace client {

    /// @brief A row of the friend list: the server's record, whether its tick box is ticked, and its online state.
    /// @details Original: TFriend_AC_ICQ; FFriendOnlineStates: 0 online, 1 busy, 2 hidden, 3 offline.
    struct Friend {
        protocol::FriendRecord record;
        bool checked = false;
        int onlineState = 3; // online state 3: offline
    };

}

#endif
