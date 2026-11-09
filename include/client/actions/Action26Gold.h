#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION26_GOLD_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION26_GOLD_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 26, the own gold (gained 26/1, spent 26/2, amount now 26/4, plus notices and the sale/ATM accounts).
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction26Gold(const core::Bytes& payload, MessageContext& context);

}

#endif
