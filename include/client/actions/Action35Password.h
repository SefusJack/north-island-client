#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION35_PASSWORD_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION35_PASSWORD_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 35, passwords: password change, character deletion, card recharge, IM points and the recommend/refill forms.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction35Password(const core::Bytes& payload, MessageContext& context);

}

#endif
