#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION53_FIGHTER_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION53_FIGHTER_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 53, what happens to a fighter (53/3 knocked off the field, 53/5 has chosen its command).
    /// @details Original: 53/3 RevFightHumDie.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction53Fighter(const core::Bytes& payload, MessageContext& context);

}

#endif
