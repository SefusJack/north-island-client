#ifndef NORTH_ISLAND_PROTOCOL_ACCOUNT_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_ACCOUNT_REQUESTS_H

#include "core/Bytes.h"

#include <string>

namespace protocol {

    /// @brief Builds client 63/2: enter the world with the slot's character, or start creating one there (the server answers 1/3 for an empty slot).
    /// @param slot Character slot (u8).
    /// @return The encoded request.
    core::Bytes selectCharacterRequest(int slot);

    /// @brief Builds client 63/0: back from character select to the login form.
    /// @return The encoded request.
    core::Bytes leaveCharacterSelectRequest();

    /// @brief Builds client 63/3: leave character creation (back to character select).
    /// @return The encoded request.
    core::Bytes cancelCreateCharacterRequest();

    /// @brief Builds client 9/2: may a new character take this name? The server answers 9/3.
    /// @param name Name as bare characters, no length byte.
    /// @return The encoded request.
    core::Bytes checkNameRequest(const std::string& name);

}

#endif
