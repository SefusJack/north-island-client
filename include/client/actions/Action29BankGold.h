#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION29_BANK_GOLD_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION29_BANK_GOLD_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 29, the bank's gold account and its forms.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction29BankGold(const core::Bytes& payload, MessageContext& context);

}

#endif
