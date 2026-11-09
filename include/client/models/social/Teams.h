#ifndef NORTH_ISLAND_CLIENT_MODELS_SOCIAL_TEAMS_H
#define NORTH_ISLAND_CLIENT_MODELS_SOCIAL_TEAMS_H

#include "protocol/SceneTeam.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace client {

    /// @brief The teams on the map: each leader with up to three members in the order they joined.
    /// @details Original: FTeamStatus 1 leader / 2 member, FTeamLeaderId, FPassengers.
    class Teams {
        public:
            static constexpr std::size_t kMostMembers = 3;

            /// @brief Adds a member to a leader's team (original RecvTeamMemberJoined).
            /// @param leaderId Leader's player id.
            /// @param memberId Joining player id.
            /// @return False when the team is full or the member already in it.
            bool join(std::int32_t leaderId, std::int32_t memberId);

            /// @brief Removes a player from his team; a leader's id ends the team (original RecvTeamDisbanded).
            /// @param playerId Leaving player id.
            /// @return Who were in the team before (leader first).
            std::vector<std::int32_t> leave(std::int32_t playerId);

            /// @brief Changes the leader: the member takes the old leader's place and the old leader the member's place among the others (original RecvTeamLeaderChanged).
            /// @param oldLeaderId Id of the old leader.
            /// @param newLeaderId Id of the member who becomes leader.
            /// @return False when the team or the member is unknown.
            bool changeLeader(std::int32_t oldLeaderId, std::int32_t newLeaderId);

            /// @brief Sets a team from the scene's team list (original RecvSceneTeams).
            /// @param team The team.
            void set(const protocol::SceneTeam& team);

            /// @brief Forgets the teams of everyone but this player's (a new map).
            /// @param playerId The own player id.
            void keepOnly(std::int32_t playerId);

            /// @brief Finds the leader of a player's team.
            /// @param playerId Player id.
            /// @return The leader's id (the player himself for a leader), nothing when not in a team.
            std::optional<std::int32_t> leaderOf(std::int32_t playerId) const;

            /// @brief Tells whether a player leads a team.
            /// @param playerId Player id.
            /// @return True for a leader.
            bool isLeader(std::int32_t playerId) const { return m_teams.count(playerId) != 0; }

            /// @brief Tells whether a player is a member (not the leader) of a team.
            /// @param playerId Player id.
            /// @return True for a member.
            bool isMember(std::int32_t playerId) const;

            /// @brief Tells whether a player is in a team.
            /// @param playerId Player id.
            /// @return True when leader or member.
            bool inTeam(std::int32_t playerId) const { return leaderOf(playerId).has_value(); }

            /// @brief Lists the members of a leader's team in order.
            /// @param leaderId Leader's id.
            /// @return The members, empty for none.
            const std::vector<std::int32_t>& members(std::int32_t leaderId) const;

            /// @brief Gives all teams.
            /// @return Map from leader id to member ids.
            const std::map<std::int32_t, std::vector<std::int32_t>>& all() const { return m_teams; }

        private:
            std::map<std::int32_t, std::vector<std::int32_t>> m_teams;
    };

}

#endif
