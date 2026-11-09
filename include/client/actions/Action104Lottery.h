#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION104_LOTTERY_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION104_LOTTERY_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 104: the online lottery.
    /// @details Original (TLa_LotteryOLForm.RecvLotteryPacket): 104/1/1 the online draw's prizes and the draws used;
    /// 104/1/2 the prize won.
    /// @param payload Raw message bytes (action, sub ids, fields).
    /// @param context Connection and event sinks of the current session.
    void handleAction104Lottery(const core::Bytes& payload, MessageContext& context);

}

#endif
