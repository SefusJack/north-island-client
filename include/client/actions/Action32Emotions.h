#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION32_EMOTIONS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION32_EMOTIONS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 32, emotions and actions played over players.
    /// @details Original: DZ_Human.pas; 32/1 RecvHumanEmotion (i32 player, u8 emotion), 32/2 RecvHumanActions (records of i32 player, u8 sprite slot).
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction32Emotions(const core::Bytes& payload, MessageContext& context);

}

#endif
