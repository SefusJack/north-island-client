#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION77_FLEE_KILL_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION77_FLEE_KILL_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 77, the flee-killing and survival event.
    /// @details Original: MZ_Menu.pas; 77/1 its notices (RecvFleeKillEvent), 77/2 the prize lists (RecvFleeKillPrizeList), 77/3 the own player's survival victories (RecvSurvivalVictory).
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction77FleeKill(const core::Bytes& payload, MessageContext& context);

}

#endif
