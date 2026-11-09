#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION50_BATTLE_ACTIONS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION50_BATTLE_ACTIONS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 50, what the fighters do (50/1: the actions played together).
    /// @details Records are decoded by protocol/BattleLists.h.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction50BattleActions(const core::Bytes& payload, MessageContext& context);

}

#endif
