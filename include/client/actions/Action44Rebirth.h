#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION44_REBIRTH_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION44_REBIRTH_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 44, pet rebirth: 44/1 the result, 44/2 opens the rebirth form.
    /// @details Original: 44/1 DZ_Connect.pas; 44/2 Se_NPCUpgradeForm.ShowCtrl.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction44Rebirth(const core::Bytes& payload, MessageContext& context);

}

#endif
