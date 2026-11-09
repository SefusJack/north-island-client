#ifndef NORTH_ISLAND_PROTOCOL_TEAM_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_TEAM_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    /// @brief Builds request 13/1: ask to join a player's team (i32 player).
    /// @param targetId Player id of the team member asked.
    /// @return The encoded request payload.
    core::Bytes joinTeamRequest(std::int32_t targetId);

    /// @brief Builds request 13/3: answer one who asked to join (u8 answer, i32 requester).
    /// @param answer 1 accept, 2 refuse, 3 no answer.
    /// @param requesterId Player id of the one who asked.
    /// @return The encoded request payload.
    core::Bytes joinAnswerRequest(int answer, std::int32_t requesterId);

    /// @brief Builds request 13/4: leave the team (i32 leader; the leader's own id disbands it).
    /// @param leaderId Player id of the team leader.
    /// @return The encoded request payload.
    core::Bytes leaveTeamRequest(std::int32_t leaderId);

    /// @brief Builds request 13/7: invite a player into the own team (i32 player).
    /// @param targetId Player id invited.
    /// @return The encoded request payload.
    core::Bytes inviteRequest(std::int32_t targetId);

    /// @brief Builds request 13/8: answer an invitation (u8 answer, i32 inviter).
    /// @param answer The answer byte.
    /// @param inviterId Player id of the inviter.
    /// @return The encoded request payload.
    core::Bytes inviteAnswerRequest(int answer, std::int32_t inviterId);

    /// @brief Builds request 13/9: the leader sends a member away (i32 member).
    /// @param memberId Player id of the member.
    /// @return The encoded request payload.
    core::Bytes kickMemberRequest(std::int32_t memberId);

    /// @brief Builds request 13/10 with u8 1: the leader asks a member to lead (i32 member).
    /// @param memberId Player id of the member.
    /// @return The encoded request payload.
    core::Bytes offerLeadRequest(std::int32_t memberId);

    /// @brief Builds request 13/10 with u8 2: the answer to a leader's offer to lead.
    /// @param answer 2 refused, 3 no answer.
    /// @return The encoded request payload.
    core::Bytes leaderAnswerRequest(int answer);

    /// @brief Builds request 13/10 with u8 3: the offer to lead is accepted.
    /// @return The encoded request payload.
    core::Bytes leaderAcceptRequest();

}

#endif
