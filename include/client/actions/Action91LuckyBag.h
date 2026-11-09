#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION91_LUCKY_BAG_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION91_LUCKY_BAG_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 91, the Lucky Bags' lists.
    /// @details Original: CJ_Menu.pas TReadLuckyBag.RecvLuckyBagPacket; 91/2 gives what a bag can hold.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction91LuckyBag(const core::Bytes& payload, MessageContext& context);

}

#endif
