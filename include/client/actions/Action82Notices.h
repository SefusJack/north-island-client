#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION82_NOTICES_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION82_NOTICES_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 82, the notice boards.
    /// @details Original: MK_Menu.pas, DZ_Connect.pas; the public board is 82/1..82/10, the guild's is 82/11..82/16.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction82Notices(const core::Bytes& payload, MessageContext& context);

}

#endif
