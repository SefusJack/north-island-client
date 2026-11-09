#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION199_GM_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION199_GM_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 199: the GM's monitor lines.
    /// @details Original (DZ_Connect.pas, CY_GM.pas TGmManage): built are the lines for the chat box: a player's local line
    /// (199/2, channel 4), a whisper (199/4, channel 3) and a world line (199/6, channel 1). The mail, player list and
    /// announcement arms write the GM's own console (TGmForm, a Windows toolkit window with no layout in the decompile)
    /// and are not built.
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction199Gm(const core::Bytes& payload, MessageContext& context);

}

#endif
