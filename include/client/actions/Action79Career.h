#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION79_CAREER_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION79_CAREER_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 79, the career.
    /// @details Original: MZ_Menu.pas TCareerForm; 79/1 opens the class form, 79/2 is the answer to the choice (1: no room for the class's cloak).
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction79Career(const core::Bytes& payload, MessageContext& context);

}

#endif
