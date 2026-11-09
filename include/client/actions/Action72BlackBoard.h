#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION72_BLACK_BOARD_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION72_BLACK_BOARD_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 72, the house blackboard.
    /// @details Original: TH_Menu.pas RecvBlackBoard; 72/1 the messages, 72/2 the refusal of a write.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction72BlackBoard(const core::Bytes& payload, MessageContext& context);

}

#endif
