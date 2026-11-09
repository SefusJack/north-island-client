#ifndef NORTH_ISLAND_PROTOCOL_ID_AND_PW_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_ID_AND_PW_REQUESTS_H

#include "core/Bytes.h"

#include <string>

namespace protocol {

    /// @brief Builds the client request sent from the second ID and password form (action 21).
    /// @param sub Form mode the server opened it in (1 or 2).
    /// @param account Account name (u8 length prefix).
    /// @param password Password (u8 length prefix).
    /// @return The encoded payload.
    core::Bytes idAndPwRequest(int sub, const std::string& account, const std::string& password);

}

#endif
