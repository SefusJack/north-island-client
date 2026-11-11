#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION39_GUILD_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION39_GUILD_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 39, guilds (messages, players' guilds, weapon material bank).
    /// @details Original: CY_Organ.pas. The messages are described in docs/protocol/guilds.md of the server.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction39Guild(const core::Bytes& payload, MessageContext& context);

}

#endif
