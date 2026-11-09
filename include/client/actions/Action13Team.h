#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION13_TEAM_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION13_TEAM_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 13: teams.
    /// @details Original (CY_Team.pas, CY_TeamMenu.pas): 13/1 i32 requester asks to join; 13/2 u8 1 the target is busy;
    /// 13/3 u8 answer, i32 leader the answer to the own request; 13/4 i32 id left or disbanded; 13/5 i32 leader, i32 member
    /// joined; 13/6 the teams on the map (TeamLists); 13/9 i32 inviter an invitation; 13/10 u8 answer, i32 invitee the
    /// answer to one; 13/13 u8 code why a team could not be formed.
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction13Team(const core::Bytes& payload, MessageContext& context);

}

#endif
