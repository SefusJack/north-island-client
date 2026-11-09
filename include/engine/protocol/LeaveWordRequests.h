#ifndef NORTH_ISLAND_PROTOCOL_LEAVE_WORD_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_LEAVE_WORD_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief Builds 36/1: leave a message in the guestbook.
    /// @param text Message text.
    /// @return The encoded payload.
    core::Bytes leaveWordSendRequest(const std::string& text);

    /// @brief Builds 36/2: take a message away.
    /// @param number Message number (u16, 1 first).
    /// @return The encoded payload.
    core::Bytes leaveWordClearRequest(std::uint16_t number);

}

#endif
