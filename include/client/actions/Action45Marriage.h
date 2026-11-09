#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION45_MARRIAGE_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION45_MARRIAGE_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 45, marriage: forwards the whole message to the marriage model.
    /// @details Original: MK_Menu.pas; applications 45/1 and 45/23, results and wedding 45/20..22 and 45/24, spouse card 45/3..10 and 45/25, notices 45/26..29.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction45Marriage(const core::Bytes& payload, MessageContext& context);

}

#endif
