#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION76_MAID_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION76_MAID_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 76, the Wit Maid's menu; the scene passes it to the maid menu while it is up.
    /// @details Original: HL_MaidMenu.pas RecvMaidAction; 76/1 the item bank, 76/2 the inn storage, 76/3 the new costume, 76/250 close.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction76Maid(const core::Bytes& payload, MessageContext& context);

}

#endif
