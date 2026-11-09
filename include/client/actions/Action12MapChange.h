#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION12_MAP_CHANGE_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION12_MAP_CHANGE_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 12 (no sub code): a player goes to another map (a door, a warp).
    /// @details Original layout: 12, i32 id, u16 map, u16 x, u16 y, u16 warp index, u8 house room, ...
    /// @param payload Raw message bytes (action, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction12MapChange(const core::Bytes& payload, MessageContext& context);

}

#endif
