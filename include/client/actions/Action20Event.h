#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION20_EVENT_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION20_EVENT_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 20: the map events (doors, NPCs, areas) the server runs.
    /// @details Original: 20/1..6 one result of the running event (20, sub, u8, u16, u8, u8 kind, u8 mode, u16 target,
    /// u8 option, i32 value, u16 data id, u8); 20/8 the event is over; 20/7 and 20/9..20/18 go on, stop waiting, wait, a notice.
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction20Event(const core::Bytes& payload, MessageContext& context);

}

#endif
