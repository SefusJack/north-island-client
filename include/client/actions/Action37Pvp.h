#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION37_PVP_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION37_PVP_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 37, the PvP and the arena: victories and ranks shown over names, arena notices, guild competition ranks.
    /// @details Original: DZ_Human.pas.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction37Pvp(const core::Bytes& payload, MessageContext& context);

}

#endif
