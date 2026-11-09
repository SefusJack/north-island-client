#ifndef NORTH_ISLAND_PROTOCOL_CHARACTER_LIST_H
#define NORTH_ISLAND_PROTOCOL_CHARACTER_LIST_H

#include "core/Bytes.h"
#include "protocol/CharacterSummary.h"

#include <optional>
#include <vector>

namespace protocol {

    /// @brief Server 63/1: the account's characters, one record each until the end of the payload.
    struct CharacterList {
        std::vector<CharacterSummary> characters;

        /// @brief Decodes the server 63/1 payload.
        /// @param payload Packet payload.
        /// @return The list, or nothing when malformed.
        static std::optional<CharacterList> decode(const core::Bytes& payload);
    };

}

#endif
