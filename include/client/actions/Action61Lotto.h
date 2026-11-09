#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION61_LOTTO_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION61_LOTTO_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 61, the Lotto (MR_Lotto.pas, DZ_Connect.pas).
    /// @details The Lotto board is never shown, so the arms that fill it (61/1-3, 61/7-9) change nothing visible; 61/4, 61/5 and 61/10 are told.
    /// @param payload Whole message payload (action and sub id first).
    /// @param context Message context with the event sink.
    void handleAction61Lotto(const core::Bytes& payload, MessageContext& context);

}

#endif
