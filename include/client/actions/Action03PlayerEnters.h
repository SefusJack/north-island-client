#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION03_PLAYER_ENTERS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION03_PLAYER_ENTERS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 3 (no sub code): a player record that enters the map.
    /// @details Original: with the own id it enters the world (OwnPlayerRecord); any other player comes into the map (OtherPlayerRecord).
    /// @param payload Raw message bytes (action, player record).
    /// @param context Connection and event sinks of the current session.
    void handleAction03PlayerEnters(const core::Bytes& payload, MessageContext& context);

}

#endif
