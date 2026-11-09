#ifndef NORTH_ISLAND_PROTOCOL_OWN_PLAYER_RECORD_H
#define NORTH_ISLAND_PROTOCOL_OWN_PLAYER_RECORD_H

#include "core/Bytes.h"
#include "protocol/PlayerAppearance.h"

#include <cstdint>
#include <optional>
#include <string>

namespace protocol {

    /// @brief The own player entering the world (server action 3 with the own player's id).
    /// @details Original: DZ_Connect action 3, own branch; 03, i32 id, u8 body type, u16 map, u16 x, u16 y (map pixels), u8 appearance,
    /// u8 head, u8 body sprite, i32 colour code 1, i32 colour code 2, u8 n + n x u16 items, u16 PvP wins, u8 PvP rank, u8 arena wins,
    /// pstr8 name, pstr8 nickname, u8 profile, u8 birth y, m, d. The record has no direction: the player stands facing south.
    struct OwnPlayerRecord {
        std::int32_t playerId = 0;
        std::uint16_t mapId = 0;
        std::uint16_t x = 0;
        std::uint16_t y = 0;
        PlayerAppearance appearance;
        int pvpWins = 0;
        int pvpRank = 0;
        int arenaVictory = 0;
        std::string name;
        std::string nickname;
        int profileChoice = 0;
        int birthYear = 0;
        int birthMonth = 0;
        int birthDay = 0;

        /// @brief Decodes the record from a payload.
        /// @param payload The packet payload.
        /// @return The record, or nothing when malformed.
        static std::optional<OwnPlayerRecord> decode(const core::Bytes& payload);
    };

}

#endif
