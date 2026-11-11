#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION63_ACCOUNT_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION63_ACCOUNT_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 63, the account after login.
    /// @details 63/1 the character list, 63/2 u32 login id.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction63Account(const core::Bytes& payload, MessageContext& context);

}

#endif
