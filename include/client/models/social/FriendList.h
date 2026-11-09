#ifndef NORTH_ISLAND_CLIENT_MODELS_SOCIAL_FRIEND_LIST_H
#define NORTH_ISLAND_CLIENT_MODELS_SOCIAL_FRIEND_LIST_H

#include "client/models/social/Friend.h"
#include "protocol/FriendRecord.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The friend list: index 0 is the GM row the game keeps first, the friends follow at 1..count (at most 50).
    /// @details Original: Friends, FriendCount.
    class FriendList {
        public:
            static constexpr int kMostFriends = 50; // most friends in the list

            /// @brief Creates the list with only the GM row (original TAC_ICQList.Create).
            FriendList();

            /// @brief Counts the friends (not the GM row).
            /// @return The number of friends.
            int count() const { return static_cast<int>(m_friends.size()) - 1; }

            /// @brief Gets a row.
            /// @param index Row 0 (GM) or 1..count.
            /// @return The row.
            const Friend& at(int index) const { return m_friends[static_cast<std::size_t>(index)]; }

            /// @brief Gets a row for change.
            /// @param index Row 0 (GM) or 1..count.
            /// @return The row.
            Friend& at(int index) { return m_friends[static_cast<std::size_t>(index)]; }

            /// @brief Finds the row of a friend (original FindFriendIndex).
            /// @param id Player id.
            /// @return The row 1..count, 0 when not a friend.
            int indexOf(std::int32_t id) const;

            /// @brief Tells whether a player is a friend.
            /// @param id Player id.
            /// @return True when a friend.
            bool isFriend(std::int32_t id) const { return indexOf(id) != 0; }

            /// @brief Replaces the whole list, keeping the online states with their places.
            /// @details Original: packet 14/5; the GM row's record comes from ids 100 / 101.
            /// @param records Records sent by the server.
            void replace(const std::vector<protocol::FriendRecord>& records);

            /// @brief Sets the GM row, updates a friend's record or adds a friend; the guild name is cleared.
            /// @details Original: packet 14/11 (RecvFriendEntry).
            /// @param record The record.
            void put(const protocol::FriendRecord& record);

            /// @brief Updates a known friend's record.
            /// @details Original: packet 14/8 (RecvFriendInfo).
            /// @param record The record.
            void update(const protocol::FriendRecord& record);

            /// @brief Adds a friend from what the world knows of it.
            /// @details Original: packet 14/3 accepted (RecvFriendAdded).
            /// @param record The record.
            void add(const protocol::FriendRecord& record);

            /// @brief Takes a friend off the list.
            /// @details Original: packet 14/4 (RecvFriendRemoved).
            /// @param id Player id.
            /// @return The name of the friend taken off, nothing if it was not one.
            std::optional<std::string> remove(std::int32_t id);

            /// @brief Sets a friend's online state.
            /// @details Original: packet 14/9.
            /// @param id Player id.
            /// @param state Online state (0 online, 1 busy, 2 hidden, 3 offline).
            void setOnlineState(std::int32_t id, int state);

            /// @brief Sets a friend's guild name.
            /// @details Original: packet 14/7.
            /// @param id Player id.
            /// @param guild Guild name.
            void setGuild(std::int32_t id, const std::string& guild);

        private:
            std::vector<Friend> m_friends;
    };

}

#endif
