#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION41_FACILITY_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION41_FACILITY_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 41, the compound facility update.
    /// @details Original: RecvCompoundFacilityUpdate, DZ_Human.pas; 41/1 sub 1 u8 count then u16 product, u16 facility; sub 2 u8 count then u16 product.
    /// @param payload Whole message payload (action byte, sub byte, body).
    /// @param context Message context with the event sink.
    void handleAction41Facility(const core::Bytes& payload, MessageContext& context);

}

#endif
