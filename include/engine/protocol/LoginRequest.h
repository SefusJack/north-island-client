#ifndef NORTH_ISLAND_PROTOCOL_LOGIN_REQUEST_H
#define NORTH_ISLAND_PROTOCOL_LOGIN_REQUEST_H

#include "core/Bytes.h"

#include <cstdint>
#include <optional>
#include <string>

namespace protocol {

    /// @brief Client 63/4: log in with account name and password.
    /// @details Original layout: 3F 04, pstr8 account, pstr8 password, u16 client version, u8 n, u8 key, n bytes of the decimal size of
    /// Data\Item.dat, each byte XOR key (n = 0 when the file is missing).
    struct LoginRequest {
        static constexpr std::uint16_t kClientVersion2014 = 1102; // client version number the 2014 client sends at login

        std::string account;
        std::string password;
        std::uint16_t clientVersion = kClientVersion2014;
        std::optional<std::uint64_t> itemDatSize;
        std::uint8_t key = 0;

        /// @brief Encodes the login packet.
        /// @return The encoded payload.
        core::Bytes encode() const;
    };

}

#endif
