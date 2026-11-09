#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION62_FURNITURE_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION62_FURNITURE_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 62, the furniture of a house; the whole message goes to the scene.
    /// @details Original: DZ_Thing.pas HandleUtensilPacket; 62/1 placed, 62/2 taken away, 62/3 moved, 62/4 the list, plus the house's own business.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction62Furniture(const core::Bytes& payload, MessageContext& context);

}

#endif
