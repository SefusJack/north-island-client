#ifndef NORTH_ISLAND_PROTOCOL_CHANGE_PASSWORD_REQUEST_H
#define NORTH_ISLAND_PROTOCOL_CHANGE_PASSWORD_REQUEST_H

#include "core/Bytes.h"

#include <string>

namespace protocol {

    /// @brief Client 35/1: change the account's password (bytes 23 01, pstr8 old password, pstr8 new password).
    struct ChangePasswordRequest {
        std::string oldPassword;
        std::string newPassword;

        /// @brief Encodes the request.
        /// @return The encoded packet.
        core::Bytes encode() const;
    };

}

#endif
