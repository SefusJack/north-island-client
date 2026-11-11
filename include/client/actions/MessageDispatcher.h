#ifndef NORTH_ISLAND_CLIENT_ACTIONS_MESSAGE_DISPATCHER_H
#define NORTH_ISLAND_CLIENT_ACTIONS_MESSAGE_DISPATCHER_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Hands a server message to the handler of its action (the first payload byte).
    /// @details Every action has its own file, ActionNNName.cpp, which looks at the sub code.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void dispatchMessage(const core::Bytes& payload, MessageContext& context);

}

#endif
