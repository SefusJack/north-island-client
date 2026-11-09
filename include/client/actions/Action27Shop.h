#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION27_SHOP_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION27_SHOP_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 27, NPC stores (purchase 27/1 and sale 27/2 results, sale forms 27/3 and 27/4).
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction27Shop(const core::Bytes& payload, MessageContext& context);

}

#endif
