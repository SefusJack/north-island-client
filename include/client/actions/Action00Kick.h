#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION00_KICK_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION00_KICK_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 0: the session ends and the second byte is the reason.
    /// @param payload Raw message bytes (action, reason).
    /// @param context Connection and event sinks of the current session.
    void handleAction00Kick(const core::Bytes& payload, MessageContext& context);

}

#endif
