#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION02_CHAT_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION02_CHAT_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 2: chat lines and chat notices.
    /// @details Original: 2/0..7, 2/11, 2/14 carry i32 speaker and text (the channel is the sub code: 0 system, 1 world,
    /// 2 local, 3 whisper, 4 GM, 5 team, 6 guild, 7 ally; 11 and 14 show as system lines); 2/12 is text from the system (speaker 0).
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction02Chat(const core::Bytes& payload, MessageContext& context);

}

#endif
