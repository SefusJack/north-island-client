#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION78_KICKED_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION78_KICKED_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 78, "Player has been kicked out".
    /// @details Original: DZ_Connect.pas, 78/1.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction78Kicked(const core::Bytes& payload, MessageContext& context);

}

#endif
