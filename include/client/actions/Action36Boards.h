#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION36_BOARDS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION36_BOARDS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 36: the announcement, leave-word, order-chart and galaxy-rank boards.
    /// @details Original: BK_Menu.pas, RE_Menu.pas; 36/7 is the leave-word refusal (DZ_Connect.pas).
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction36Boards(const core::Bytes& payload, MessageContext& context);

}

#endif
