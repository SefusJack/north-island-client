#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION10_NAMES_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION10_NAMES_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 10: nicknames and heart numbers.
    /// @details Original (DZ_Connect.pas): 10/1 i32 player, pstr8 nickname; 10/2 and 10/3 i32 player, u8 heart number.
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction10Names(const core::Bytes& payload, MessageContext& context);

}

#endif
