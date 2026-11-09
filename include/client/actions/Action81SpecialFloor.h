#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION81_SPECIAL_FLOOR_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION81_SPECIAL_FLOOR_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 81, the battle time limit and the special house room.
    /// @details Original: DZ_Connect.pas; 81/3 starts the battle's 3-minute limit; 81/1, 81/2 and 81/4 are the thing of a special house room.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction81SpecialFloor(const core::Bytes& payload, MessageContext& context);

}

#endif
