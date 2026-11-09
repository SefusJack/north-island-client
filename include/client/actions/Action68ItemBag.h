#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION68_ITEM_BAG_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION68_ITEM_BAG_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 68, the Item Bag (TRe_MiniThingGridForm).
    /// @details 68/1, 68/2, 68/3 (u8 a, u8 b, u8 count): the order the client sent is done (1 cell a to bag place b, 2 cell a to cell b, 3 bag place a to cell b).
    /// 68/4: the whole grid, 29-byte records like the bag's (u8 cell first); 68/9: show the form; 68/5-68/8 are ignored.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction68ItemBag(const core::Bytes& payload, MessageContext& context);

}

#endif
