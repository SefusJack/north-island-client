#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION75_MALL_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION75_MALL_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 75, the item mall.
    /// @details 75/1 the catalog; 75/2 the catalog again (changed); 75/3 (i32 points left, i32 spent, u16 item, u8 count);
    /// 75/4 (u16 item, u8 category, u8 count, u16 key, u8 1 bought / 0 failed); 75/5 (u8 why a purchase was refused);
    /// 75/6 (u8 how the forge went: 1 = u8 bag place, u8 fusion level; 3 = u8 bag place); 75/7 (u8 1: the slot machines' tab opens).
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction75Mall(const core::Bytes& payload, MessageContext& context);

}

#endif
