#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION67_PRIZES_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION67_PRIZES_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 67, prize and pet exchange refusals and forgotten skills.
    /// @details Original: DZ_Connect.pas; 67/2, 67/3, 67/4 refusals, 67/5 the skills a player forgot.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction67Prizes(const core::Bytes& payload, MessageContext& context);

}

#endif
