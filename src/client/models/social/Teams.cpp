#include "client/models/social/Teams.h"

#include <algorithm>

namespace client {

    bool Teams::join(std::int32_t leaderId, std::int32_t memberId) {
        std::vector<std::int32_t>& team = m_teams[leaderId];
        if (team.size() >= kMostMembers || std::find(team.begin(), team.end(), memberId) != team.end()) {
            if (team.empty()) {
                m_teams.erase(leaderId);
            }
            return false;
        }
        team.push_back(memberId);
        return true;
    }

    std::vector<std::int32_t> Teams::leave(std::int32_t playerId) {
        const auto leader = leaderOf(playerId);
        if (!leader) {
            return {};
        }
        std::vector<std::int32_t> before{*leader};
        std::vector<std::int32_t>& team = m_teams[*leader];
        before.insert(before.end(), team.begin(), team.end());
        if (playerId == *leader) {
            m_teams.erase(*leader);
            return before;
        }
        team.erase(std::remove(team.begin(), team.end(), playerId), team.end());
        if (team.empty()) {
            m_teams.erase(*leader);
        }
        return before;
    }

    bool Teams::changeLeader(std::int32_t oldLeaderId, std::int32_t newLeaderId) {
        const auto found = m_teams.find(oldLeaderId);
        if (found == m_teams.end()) {
            return false;
        }
        std::vector<std::int32_t> team = found->second;
        const auto place = std::find(team.begin(), team.end(), newLeaderId);
        if (place == team.end()) {
            return false;
        }
        *place = oldLeaderId;
        m_teams.erase(found);
        m_teams[newLeaderId] = team;
        return true;
    }

    void Teams::set(const protocol::SceneTeam& team) {
        if (team.memberIds.empty()) {
            return;
        }
        std::vector<std::int32_t> members = team.memberIds;
        if (members.size() > kMostMembers) {
            members.resize(kMostMembers);
        }
        m_teams[team.leaderId] = members;
    }

    void Teams::keepOnly(std::int32_t playerId) {
        const auto leader = leaderOf(playerId);
        for (auto it = m_teams.begin(); it != m_teams.end();) {
            it = leader && it->first == *leader ? std::next(it) : m_teams.erase(it);
        }
    }

    std::optional<std::int32_t> Teams::leaderOf(std::int32_t playerId) const {
        if (isLeader(playerId)) {
            return playerId;
        }
        for (const auto& [leader, team] : m_teams) {
            if (std::find(team.begin(), team.end(), playerId) != team.end()) {
                return leader;
            }
        }
        return std::nullopt;
    }

    bool Teams::isMember(std::int32_t playerId) const {
        const auto leader = leaderOf(playerId);
        return leader && *leader != playerId;
    }

    const std::vector<std::int32_t>& Teams::members(std::int32_t leaderId) const {
        static const std::vector<std::int32_t> none;
        const auto found = m_teams.find(leaderId);
        return found != m_teams.end() ? found->second : none;
    }

}
