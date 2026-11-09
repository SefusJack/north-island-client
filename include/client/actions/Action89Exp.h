#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION89_EXP_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION89_EXP_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 89, the exp an event gave the player.
    /// @details Original: DZ_Connect.pas; 89/1/1 is an i32 exp, told in the chat with "ExpUP" over the player.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction89Exp(const core::Bytes& payload, MessageContext& context);

}

#endif
