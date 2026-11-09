#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION09_CREATE_CHARACTER_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION09_CREATE_CHARACTER_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 9: character creation.
    /// @details Original: 9/3 u8 code is the answer to the name the wizard asked about (0 accepted, 1 taken, 2 invalid).
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction09CreateCharacter(const core::Bytes& payload, MessageContext& context);

}

#endif
