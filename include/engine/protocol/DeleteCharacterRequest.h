#ifndef NORTH_ISLAND_PROTOCOL_DELETE_CHARACTER_REQUEST_H
#define NORTH_ISLAND_PROTOCOL_DELETE_CHARACTER_REQUEST_H

#include "core/Bytes.h"

#include <string>

namespace protocol {

    /// @brief Client 35/2: delete the slot's character (bytes 23 02, u8 slot, pstr8 password, pstr8 cipher).
    struct DeleteCharacterRequest {
        int slot = 0;
        std::string password;
        std::string cipher;

        /// @brief Encodes the request.
        /// @return The encoded packet.
        core::Bytes encode() const;
    };

}

#endif
