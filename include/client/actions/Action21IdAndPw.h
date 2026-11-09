#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION21_ID_AND_PW_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION21_ID_AND_PW_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 21: the second ID and password form.
    /// @details Original (RE_Menu.pas TRe_IdAndPwForm; DZ_Connect.pas): 21/1 and 21/2 open it in that mode; 21/3 says the
    /// account or password was refused.
    /// @param payload Raw message bytes (action, sub id, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction21IdAndPw(const core::Bytes& payload, MessageContext& context);

}

#endif
