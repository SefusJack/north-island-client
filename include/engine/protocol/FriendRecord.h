#ifndef NORTH_ISLAND_PROTOCOL_FRIEND_RECORD_H
#define NORTH_ISLAND_PROTOCOL_FRIEND_RECORD_H

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief A friend as the server describes it (14/5, 14/8, 14/11): who, level, job, reborn, element, face (body type, head, two colour codes), nickname and guild name.
    /// @details Original: TFriend_AC_ICQ.
    struct FriendRecord {
        std::int32_t id = 0;
        std::string name;
        int level = 0;
        int job = 0;
        int reborn = 0;
        int element = 0;
        int bodyType = 0;
        int head = 0;
        std::int32_t colorCode1 = 0;
        std::int32_t colorCode5 = 0;
        std::string nickname;
        std::string guild;
    };

}

#endif
