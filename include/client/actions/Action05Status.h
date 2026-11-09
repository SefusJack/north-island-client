#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION05_STATUS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION05_STATUS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 5: the own player's status.
    /// @details Original: 5/3 all of the own player's attributes (OwnAttributes); 5/4 the player is in the world
    /// (players are drawn and may walk from now on).
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction05Status(const core::Bytes& payload, MessageContext& context);

}

#endif
