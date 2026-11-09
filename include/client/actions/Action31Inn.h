#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION31_INN_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION31_INN_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 31, the inn: resting and the pet storage (records decoded by protocol/PetLists.h).
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction31Inn(const core::Bytes& payload, MessageContext& context);

}

#endif
