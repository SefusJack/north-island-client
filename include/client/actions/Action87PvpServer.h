#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION87_PVP_SERVER_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION87_PVP_SERVER_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 87, the move to the PvP server.
    /// @details 87/2 is the refusal (DZ_Connect.pas); 87/3 (RecvServerCode) is ignored (the PvP Server's "N-Name" is not built); 87/1 (log out and come back to the other server) is not built.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction87PvpServer(const core::Bytes& payload, MessageContext& context);

}

#endif
