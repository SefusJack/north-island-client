#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION22_MAP_NPC_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION22_MAP_NPC_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 22: the map's NPCs, addressed by their eve.Emg index.
    /// @details Original: 22/1 u16 index, u8 frame a scene object shows that frame still (255: animates); 22/2 u16 index,
    /// u16 x, u16 y, u8 speed walks there; 22/5 u16 index, u16 x, u16 y stands there; 22/6 u16 index, u8 state shown unless
    /// the state is 2 or 4; 22/10 u16 index, u16 seconds hidden; 22/11 u16 index, u16 seconds shown; 22/3 the same as 22/10;
    /// 22/7 u16 index, u8 emotion shows emotion 100 + n; 22/8 u16 index, u8 action (22/9 and u8 direction) plays an action;
    /// 22/12 u8 kind (1 NPC, 2 door, 3 mine), u16 index, u8 sign is the sign on the small map.
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction22MapNpc(const core::Bytes& payload, MessageContext& context);

}

#endif
