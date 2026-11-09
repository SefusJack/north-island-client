#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION69_PVP_EVENTS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION69_PVP_EVENTS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 69, the notices of the PvP events.
    /// @details Original: DZ_Human.pas ShowPvpEventNotice (69/1), RecvPrizeLists (69/2), Show4PlayerPkNotice (69/3).
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction69PvpEvents(const core::Bytes& payload, MessageContext& context);

}

#endif
