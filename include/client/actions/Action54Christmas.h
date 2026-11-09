#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION54_CHRISTMAS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION54_CHRISTMAS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 54, the Christmas event beginning and ending.
    /// @details Original: DZ_Connect.pas; 54/1 and 54/2 say so in the chat and turn it on or off, 54/3 and 54/4 only turn it on or off, 54/5 and 54/6 only say so.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction54Christmas(const core::Bytes& payload, MessageContext& context);

}

#endif
