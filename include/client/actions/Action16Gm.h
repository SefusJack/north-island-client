#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION16_GM_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION16_GM_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 16: the GM's tools, only the parts a player sees.
    /// @details Original (DZ_Connect.pas, CY_GM.pas): built are the GM's question (16/1), the notice of its answer (16/2),
    /// a GM silencing channels (16/7) and releasing them (16/8, 16/13), "You talk too often" (16/20) and a GM's system line
    /// (16/18). The rest write the GM's own console (TGmForm, a Windows toolkit window with no layout in the decompile)
    /// and are not built.
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction16Gm(const core::Bytes& payload, MessageContext& context);

}

#endif
