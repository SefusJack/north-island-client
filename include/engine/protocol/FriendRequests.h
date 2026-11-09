#ifndef NORTH_ISLAND_PROTOCOL_FRIEND_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_FRIEND_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>
#include <string>
#include <vector>

namespace protocol {

    /// @brief Builds client 14/2: ask a player to be friends (original AddFriendFromScene).
    /// @param playerId Player id (i32).
    /// @return The encoded request.
    core::Bytes addFriendRequest(std::int32_t playerId);

    /// @brief Builds client 14/3: answer a player who asked to be friends.
    /// @param playerId Player id (i32).
    /// @param answer 1 yes, 2 no, 3 no answer.
    /// @return The encoded request.
    core::Bytes friendAnswerRequest(std::int32_t playerId, int answer);

    /// @brief Builds client 14/4: take a friend off the list.
    /// @param friendId Friend's player id (i32).
    /// @return The encoded request.
    core::Bytes deleteFriendRequest(std::int32_t friendId);

    /// @brief Builds client 14/1: mail (u8 count, i32 player per receiver, then the text with no length; rows end with a carriage return).
    /// @param receivers Receiving player ids.
    /// @param text Mail text.
    /// @return The encoded request.
    core::Bytes mailRequest(const std::vector<std::int32_t>& receivers, const std::string& text);

    /// @brief Builds client 14/6: mail to a player by his name (original SendFurMail).
    /// @param ownId Own player id (i32).
    /// @param name Receiver's name (pstr8).
    /// @param text Mail text.
    /// @return The encoded request.
    core::Bytes furMailRequest(std::int32_t ownId, const std::string& name, const std::string& text);

    /// @brief Builds client 14/5: change the black list.
    /// @param change 1 add, 2 remove.
    /// @param playerId Player id (i32).
    /// @param name Player name (pstr8).
    /// @return The encoded request.
    core::Bytes blacklistRequest(int change, std::int32_t playerId, const std::string& name);

}

#endif
