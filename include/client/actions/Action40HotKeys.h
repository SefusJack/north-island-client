#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION40_HOT_KEYS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION40_HOT_KEYS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 40, the F1-F8 bar's places (40/1: u8 kind, u16 id, u8 page, u8 place each).
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction40HotKeys(const core::Bytes& payload, MessageContext& context);

}

#endif
