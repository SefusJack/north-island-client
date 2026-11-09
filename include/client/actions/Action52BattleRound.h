#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION52_BATTLE_ROUND_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION52_BATTLE_ROUND_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 52, the rounds of a battle (52/1: the fighters choose their commands).
    /// @details Original: RevCommandRoundStart.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction52BattleRound(const core::Bytes& payload, MessageContext& context);

}

#endif
