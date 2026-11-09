#ifndef NORTH_ISLAND_CLIENT_RULES_TARGET_NAMES_H
#define NORTH_ISLAND_CLIENT_RULES_TARGET_NAMES_H

#include "client/models/social/FriendList.h"
#include "client/models/guild/Guild.h"

#include <cstdint>
#include <string>
#include <vector>

namespace client {

    /// @brief Lists the player names a target form offers.
    /// @details Original: FillMagnetTargetList, FillBlessTargetList, FillFlowerTargetList. Friends first when their tick is on, then guild members when theirs is; each name once, the player himself not among the members.
    /// @param friends The friend list.
    /// @param guild The guild.
    /// @param ownId Id of the own player.
    /// @param withFriends True to include the friends.
    /// @param withGuild True to include the guild members.
    /// @return The names.
    std::vector<std::string> targetNames(
        const FriendList& friends,
        const Guild& guild,
        std::int32_t ownId,
        bool withFriends,
        bool withGuild
    );

}

#endif
