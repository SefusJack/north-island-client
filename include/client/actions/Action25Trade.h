#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION25_TRADE_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION25_TRADE_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 25: item, pet and utensil trades, flowers, express pets.
    /// @details Original: CY_Trade.pas, CY_TradeMenu.pas; 25/1..3 item trade, 25/10..12 pet trade, 25/43 refusal, 25/44 online time.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction25Trade(const core::Bytes& payload, MessageContext& context);

}

#endif
