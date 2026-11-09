#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION19_ACTIVE_PET_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION19_ACTIVE_PET_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 19: the pet that fights.
    /// @details Original: 19/1 i32 npc is this one (RecvActivePet); 19/2 none (ClearActivePet); 19/4 i32 npc is this one at
    /// login (RecvSelectedPet); 19/7 i32 player another's pet rests.
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction19ActivePet(const core::Bytes& payload, MessageContext& context);

}

#endif
