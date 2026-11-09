#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION11_BATTLE_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION11_BATTLE_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 11: battles starting and ending.
    /// @details Original: 11/250 the battle ground, the event's battle and the fighters (protocol/BattleLists.h); 11/10 u8
    /// the battle's type (2, 4, 7, 8, 9: 30 seconds to command, else 20); 11/1 u8 row, u8 col that fighter runs off
    /// (the own one: the battle is over for them).
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction11Battle(const core::Bytes& payload, MessageContext& context);

}

#endif
