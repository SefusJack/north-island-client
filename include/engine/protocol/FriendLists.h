#ifndef NORTH_ISLAND_PROTOCOL_FRIEND_LISTS_H
#define NORTH_ISLAND_PROTOCOL_FRIEND_LISTS_H

#include "core/Bytes.h"
#include "protocol/BlacklistEntry.h"
#include "protocol/FriendRecord.h"
#include "protocol/FriendState.h"

#include <optional>
#include <vector>

namespace protocol {

    /// @brief Decodes server 14/5: the whole friend list (ids 100 and 101 are the GM row).
    /// @param payload Packet payload.
    /// @return The friends.
    std::vector<FriendRecord> decodeFriendList(const core::Bytes& payload);

    /// @brief Decodes server 14/8: a friend's record with its guild.
    /// @details Original: RecvFriendInfo.
    /// @param payload Packet payload.
    /// @return The friend, or nothing when malformed.
    std::optional<FriendRecord> decodeFriendInfo(const core::Bytes& payload);

    /// @brief Decodes server 14/11: a friend added or updated, without the guild.
    /// @details Original: RecvFriendEntry.
    /// @param payload Packet payload.
    /// @return The friend, or nothing when malformed.
    std::optional<FriendRecord> decodeFriendEntry(const core::Bytes& payload);

    /// @brief Decodes server 14/9: records of i32 friend, u8 state.
    /// @details Original: ParseFriendOnlineStates.
    /// @param payload Packet payload.
    /// @return The friend states.
    std::vector<FriendState> decodeFriendStates(const core::Bytes& payload);

    /// @brief Decodes server 14/13 after its result byte: records of i32 player, pstr8 name.
    /// @param payload Packet payload.
    /// @return The black list entries.
    std::vector<BlacklistEntry> decodeBlacklist(const core::Bytes& payload);

}

#endif
