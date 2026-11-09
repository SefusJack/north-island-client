#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION43_ALLIANCE_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION43_ALLIANCE_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 43, the guild alliances (request, answer, disband, list, left, Trojan War refusals).
    /// @details Original: CY_Organ.pas, DZ_Connect.pas.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction43Alliance(const core::Bytes& payload, MessageContext& context);

}

#endif
