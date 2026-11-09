#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION33_SWITCHES_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION33_SWITCHES_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 33, the system window's switches (PK, fighting, trade, party, chat channels).
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction33Switches(const core::Bytes& payload, MessageContext& context);

}

#endif
