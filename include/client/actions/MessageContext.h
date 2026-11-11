#ifndef NORTH_ISLAND_CLIENT_ACTIONS_MESSAGE_CONTEXT_H
#define NORTH_ISLAND_CLIENT_ACTIONS_MESSAGE_CONTEXT_H

#include "client/services/ServerEvents.h"
#include "net/GameConnection.h"

#include <cstdint>

namespace client {

    /// @brief What an action handler works with: the event sink, the connection and the own player's id (the account's login id, 63/2).
    struct MessageContext {
        ServerEvents& events;
        net::GameConnection& connection;
        std::uint32_t ownPlayerId = 0;
    };

}

#endif
