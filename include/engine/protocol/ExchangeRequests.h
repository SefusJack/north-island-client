#ifndef NORTH_ISLAND_PROTOCOL_EXCHANGE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_EXCHANGE_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    /// @brief Builds client 23/109: exchange IM points for gold at the ATM.
    /// @param points Number of IM points (i32).
    /// @return The encoded request.
    core::Bytes pointExchangeRequest(std::int32_t points);

}

#endif
