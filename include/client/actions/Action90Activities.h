#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION90_ACTIVITIES_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION90_ACTIVITIES_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 90, the event schedule.
    /// @details Original: TActionListForm.RecvActionPacket; 90/0 the schedule, 90/1 the states of its events, 90/2 a notice (0 the server is busy, 1 the schedule was updated).
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction90Activities(const core::Bytes& payload, MessageContext& context);

}

#endif
