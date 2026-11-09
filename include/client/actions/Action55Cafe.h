#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION55_CAFE_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION55_CAFE_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 55, the internet cafe sign-in (55/1 opens the number form, 55/2 is the answer).
    /// @details Original: DZ_Connect.pas; the form is Se_CafeIDForm; a refused number opens the form again.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction55Cafe(const core::Bytes& payload, MessageContext& context);

}

#endif
