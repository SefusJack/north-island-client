#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION66_REBORN_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION66_REBORN_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 66, the Wishing God's wish and the time effect list.
    /// @details Original: wish 66/11..66/16 in RE_Menu.pas TRe_RebornForm; time effects 66/1, 66/2 in DZ_Human.pas HandleTimeEffectListPacket.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction66Reborn(const core::Bytes& payload, MessageContext& context);

}

#endif
