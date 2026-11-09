#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION64_MANUFACTURE_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION64_MANUFACTURE_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 64, the manufacturing of the player's house; the whole message goes to the scene.
    /// @details Original: DZ_Thing.pas HandleMakePacket; 64/1 the jobs, 64/2 started, 64/3 stopped, 64/4 finished, 64/5 given up, 64/6..8 one that cannot finish, 64/10 the pet that makes it.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction64Manufacture(const core::Bytes& payload, MessageContext& context);

}

#endif
