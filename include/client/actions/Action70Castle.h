#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION70_CASTLE_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION70_CASTLE_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 70, the castle and the Trojan war.
    /// @details Original: RecvTrojanWarMsg. 70/1 u8 message: most are a line for the message box or the chat; 10 (u8 result, pstr8 guild, i32 ensign) is the new leader of the city; 23 (pstr8 guild, i32 ensign) is who owns the castle.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction70Castle(const core::Bytes& payload, MessageContext& context);

}

#endif
