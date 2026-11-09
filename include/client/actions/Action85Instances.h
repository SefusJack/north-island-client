#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION85_INSTANCES_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION85_INSTANCES_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 85, the instances ("duplicate missions"); the whole message goes to the instance model.
    /// @details Original: MZ_Menu.pas. 85/1, 85/8, 85/11 the list of open ones; 85/2, 85/3 the answers to creating and joining; 85/4 the detail window;
    /// 85/5, 85/6, 85/7, 85/9, 85/10, 85/12, 85/13 the player's own instance; 85/14 which instances may be created.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction85Instances(const core::Bytes& payload, MessageContext& context);

}

#endif
