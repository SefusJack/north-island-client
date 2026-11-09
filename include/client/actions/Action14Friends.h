#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION14_FRIENDS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION14_FRIENDS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 14: friends and mail.
    /// @details Original (AC_ICQ.pas): 14/1 i32 sender, f64 time, text is mail; 14/6 the receiver's mail box is full;
    /// 14/10 too much mail sent; 14/2 i32 player asks to be friends; 14/3 i32 player, u8 answer the answer to the own
    /// request; 14/4 i32 friend left the list; 14/5 the list (FriendLists); 14/7 i32 player, pstr8 guild; 14/8 a friend's
    /// record; 14/9 online states; 14/11 a friend's record (added when new); 14/12 the asked player's list is full;
    /// 14/13 u8 result (0 failed, 1 added, 2 removed, 3 the list) and black list records; 14/14 u8 code the player asked
    /// has you on their black list.
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction14Friends(const core::Bytes& payload, MessageContext& context);

}

#endif
