#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION86_PROPERTY_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION86_PROPERTY_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 86, the lost property office.
    /// @details Original: DZ_Connect.pas; 86/1 the list, 86/2 a piece reclaimed, 86/3 a refusal.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction86Property(const core::Bytes& payload, MessageContext& context);

}

#endif
