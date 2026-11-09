#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION65_TENTS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION65_TENTS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 65, tents; the whole message goes to the scene.
    /// @details Original: Re_Camp.pas; 65/1 pitched, 65/3 listed, 65/4 taken back, 65/14 restyled, 65/2, 65/5, 65/6, 65/8 limits and refusals, 65/7 and 65/11..13 the tent's house.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction65Tents(const core::Bytes& payload, MessageContext& context);

}

#endif
