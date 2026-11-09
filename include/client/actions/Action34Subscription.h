#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION34_SUBSCRIPTION_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION34_SUBSCRIPTION_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 34, the IM points notice (34/1).
    /// @details Original: DZ_Connect.pas. 34/2 (RecvMonthlySubscription, the billing form Tjo_Charge) is not built.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction34Subscription(const core::Bytes& payload, MessageContext& context);

}

#endif
