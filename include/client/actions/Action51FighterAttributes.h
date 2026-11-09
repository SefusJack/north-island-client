#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION51_FIGHTER_ATTRIBUTES_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION51_FIGHTER_ATTRIBUTES_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 51, fighters' values (51/1: records of u8 row, u8 col, u8 attribute, i32 value).
    /// @details Original: RevFightHumAttrs.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction51FighterAttributes(const core::Bytes& payload, MessageContext& context);

}

#endif
