#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION01_SYSTEM_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION01_SYSTEM_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 1: connection and login results.
    /// @details Original sub ids: 1/1 i32 id a player left the map; 1/3 u8 hasCipher opens character creation; 1/5 time out;
    /// 1/6 wrong account or password; 1/7 connection failed; 1/8 u32 seconds a character is locked; 1/9 server accepted the
    /// connection; 1/10 u32 seconds the account is locked; 1/11 the map is ready, show the world.
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction01System(const core::Bytes& payload, MessageContext& context);

}

#endif
