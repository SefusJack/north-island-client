#ifndef NORTH_ISLAND_PROTOCOL_CAFE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_CAFE_REQUESTS_H

#include "core/Bytes.h"

#include <string>

namespace protocol {

    /// @brief Builds client 55/1: the internet cafe's number as typed (no length byte).
    /// @param number The cafe number text.
    /// @return The encoded request.
    core::Bytes cafeNumberRequest(const std::string& number);

}

#endif
