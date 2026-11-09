#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION83_PET_VOUCHER_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION83_PET_VOUCHER_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 83, pet egg and pet exchange voucher answers.
    /// @details Original: DZ_Connect.pas; 83/1 to 83/5 the refusals, 83/6 the item it gave.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction83PetVoucher(const core::Bytes& payload, MessageContext& context);

}

#endif
