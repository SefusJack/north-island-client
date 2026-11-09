#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION07_JUMP_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION07_JUMP_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 7 (no sub code): a player is put on another place of the map.
    /// @details Original layout: 07, i32 id, u16 map, u16 x, u16 y, [u8 house room].
    /// @param payload Raw message bytes (action, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction07Jump(const core::Bytes& payload, MessageContext& context);

}

#endif
