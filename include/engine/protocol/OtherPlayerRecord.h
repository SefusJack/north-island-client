#ifndef NORTH_ISLAND_PROTOCOL_OTHER_PLAYER_RECORD_H
#define NORTH_ISLAND_PROTOCOL_OTHER_PLAYER_RECORD_H

#include "core/Bytes.h"
#include "protocol/PlayerAppearance.h"

#include <cstdint>
#include <optional>
#include <string>

namespace protocol {

    /// @brief Another player entering (server action 3, with an effect) or already there (action 4).
    /// @details Original: AddWorldPlayer; 03|04, i32 id, u8 body type, u8 element, u8 level, u16 map, u16 x, u16 y, u8 appearance, u8 head,
    /// u8 body sprite, i32 colour code 1, i32 colour code 2, u8 n + n x u16 items, u16 PvP wins, u8 rank, u8 online, u8 arena, u8 reborn,
    /// u8 job, pstr8 name, pstr8 nickname, u8 heart.
    struct OtherPlayerRecord {
        std::int32_t playerId = 0;
        int element = 0;
        int level = 0;
        std::uint16_t mapId = 0;
        std::uint16_t x = 0;
        std::uint16_t y = 0;
        PlayerAppearance appearance;
        int pvpWins = 0;
        int pvpRank = 0;
        int arenaVictory = 0;
        int reborn = 0;
        int job = 0;
        int onlineStatus = 0;
        std::string name;
        std::string nickname;
        int heartNumber = 0;

        /// @brief Decodes the record from a payload.
        /// @param payload The packet payload.
        /// @return The record, or nothing when malformed.
        static std::optional<OtherPlayerRecord> decode(const core::Bytes& payload);
    };

}

#endif
