#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION30_BANK_ITEMS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION30_BANK_ITEMS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 30, the bank's items (records decoded by protocol/BankLists.h).
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction30BankItems(const core::Bytes& payload, MessageContext& context);

}

#endif
