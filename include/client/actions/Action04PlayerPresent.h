#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION04_PLAYER_PRESENT_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION04_PLAYER_PRESENT_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 4 (no sub code): a player who is already on the map.
    /// @details Original: an OtherPlayerRecord, shown without the entering effect.
    /// @param payload Raw message bytes (action, player record).
    /// @param context Connection and event sinks of the current session.
    void handleAction04PlayerPresent(const core::Bytes& payload, MessageContext& context);

}

#endif
