#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION08_ATTRIBUTE_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION08_ATTRIBUTE_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 8: the own player's attributes one at a time.
    /// @details Original: 8/1 u8 attribute, u8 sign (2: negative), i32 value, i32 second value; 8/2 u8 4, u16 pet place,
    /// u8 attribute, u8 sign, i32 value, i32 second value (an own pet); 8/3 i32 player, u8 attribute, u8 sign, i32 value,
    /// i32 second value (a teammate's HP, SP and what the maxima need).
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction08Attribute(const core::Bytes& payload, MessageContext& context);

}

#endif
