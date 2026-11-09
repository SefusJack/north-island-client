#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION24_MARKS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION24_MARKS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 24, the player's quest marks (add, remove, delete, flag bits, slot records).
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction24Marks(const core::Bytes& payload, MessageContext& context);

}

#endif
