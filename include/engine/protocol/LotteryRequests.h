#ifndef NORTH_ISLAND_PROTOCOL_LOTTERY_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_LOTTERY_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 104/1: the draw's animation is over, the server gives the prize.
    /// @return The encoded payload.
    core::Bytes lotteryDrawRequest();

}

#endif
