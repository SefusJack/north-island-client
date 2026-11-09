#ifndef NORTH_ISLAND_PROTOCOL_CREATE_CHARACTER_REQUEST_H
#define NORTH_ISLAND_PROTOCOL_CREATE_CHARACTER_REQUEST_H

#include "core/Bytes.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>

namespace protocol {

    /// @brief Client 9/1: create the character the wizard put together (the name was accepted by 9/2 before).
    /// @details Layout: 09 01, u8 body type, u8 0, u8 head, u8 body sprite, i32 colour code 1, i32 colour code 2, u8 element, u8 STR, AGI, WIS, INT, CON, and the cipher as pstr8 when the account has none yet.
    struct CreateCharacterRequest {
        int bodyType = 1;
        int head = 0;
        int bodySprite = 0;
        std::int32_t colorCode1 = 0;
        std::int32_t colorCode2 = 0;
        int element = 1;
        std::array<int, 5> stats{};
        std::optional<std::string> cipher;

        /// @brief Encodes the request.
        /// @return The encoded packet.
        core::Bytes encode() const;
    };

}

#endif
