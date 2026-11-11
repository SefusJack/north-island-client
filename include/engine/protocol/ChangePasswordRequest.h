#ifndef NORTH_ISLAND_PROTOCOL_CHANGE_PASSWORD_REQUEST_H
#define NORTH_ISLAND_PROTOCOL_CHANGE_PASSWORD_REQUEST_H

#include "core/Bytes.h"

#include <string>

namespace protocol {

    /// @brief Client 35/1: change the secret code (bytes 23 01, pstr8 old code, pstr8 new code).
    struct ChangePasswordRequest {
        std::string oldPassword;
        std::string newPassword;

        /// @brief Encodes the request.
        /// @return The encoded packet.
        core::Bytes encode() const;
    };

    /// @brief Client 35/12: change the account's password (bytes 23 0C, pstr8 old password, pstr8 new password).
    struct ChangeAccountPasswordRequest {
        std::string oldPassword;
        std::string newPassword;

        /// @brief Encodes the request.
        /// @return The encoded packet.
        core::Bytes encode() const;
    };

}

#endif
