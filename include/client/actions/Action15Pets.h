#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION15_PETS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION15_PETS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 15: pets.
    /// @details Original (CY_Team.pas): 15/1 a pet joined (PetLists); 15/2 i32 owner, u8 place a pet left; 15/4 other
    /// players' fighting pets (PetLists); 15/8 the own pets (PetLists); 15/9 i32 owner, u8 place, name renamed.
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction15Pets(const core::Bytes& payload, MessageContext& context);

}

#endif
