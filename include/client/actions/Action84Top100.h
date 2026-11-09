#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION84_TOP100_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION84_TOP100_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 84, the Top 100 window.
    /// @details Original: LD_Top100.pas; 84/1 shows the window, 84/2 is one of its lists.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction84Top100(const core::Bytes& payload, MessageContext& context);

}

#endif
