#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION71_MACHINES_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION71_MACHINES_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 71, the prize machines' answers; the whole message goes to the scene.
    /// @details The Lucky Wheel's answer is 71/21.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction71Machines(const core::Bytes& payload, MessageContext& context);

}

#endif
