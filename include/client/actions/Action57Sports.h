#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION57_SPORTS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION57_SPORTS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 57, the mini games (start 57/1, end 57/2, Lucky Wheel list and answers 57/12).
    /// @details Original: CY_Sport.pas TSportManage.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction57Sports(const core::Bytes& payload, MessageContext& context);

}

#endif
