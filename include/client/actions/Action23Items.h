#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION23_ITEMS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION23_ITEMS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 23, the own items: bag, equipment, stalls, ground items, warehouse, sale store and many item effects.
    /// @details Original: see the sub id list above (23/5 bag, 23/8 added, 23/11 worn, 23/30..34 item stalls, 23/110..114 pet stalls, 23/4 ground records, 23/43..46 warehouse, 23/60..65 sale store).
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction23Items(const core::Bytes& payload, MessageContext& context);

}

#endif
