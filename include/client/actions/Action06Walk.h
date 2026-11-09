#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION06_WALK_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION06_WALK_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 6: players walking.
    /// @details Original: 6/1 i32 id, u8 direction, u16 x, u16 y walks there and then faces that way; 6/2 u8 lock the own
    /// player stops (and may not walk while lock is not 0); 6/3 i32 id, u16 x, u16 y walks there.
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction06Walk(const core::Bytes& payload, MessageContext& context);

}

#endif
