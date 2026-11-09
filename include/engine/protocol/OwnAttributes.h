#ifndef NORTH_ISLAND_PROTOCOL_OWN_ATTRIBUTES_H
#define NORTH_ISLAND_PROTOCOL_OWN_ATTRIBUTES_H

#include "core/Bytes.h"
#include "protocol/LearnedSkill.h"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace protocol {

    /// @brief The own player's attributes (server 5/3, 2014 layout).
    /// @details Original: RecvPlayerAttributes; 05 03, u8 element, i32 HP, u16 SP, u16 STR, CON, INT, WIS, AGI, u8 level, i32 exp,
    /// u16 skill points, u16 stat points, i32 max HP, u16 max SP, 6 x i32 bonuses, u32 guild, u16 n + n x (u16 skill key, u8 level, i32 exp),
    /// u16, u16, u8 reborn, u8 job, u8 potential.
    struct OwnAttributes {
        int element = 0;
        std::int32_t hp = 0;
        int sp = 0;
        std::array<int, 5> stats{};
        int level = 0;
        std::int32_t exp = 0;
        int skillPoints = 0;
        int statPoints = 0;
        std::int32_t maxHp = 0;
        int maxSp = 0;
        int reborn = 0;
        int job = 0;
        int potential = 0;
        std::uint32_t guildId = 0;
        std::vector<LearnedSkill> skills;

        /// @brief Decodes the attributes from a payload.
        /// @param payload The packet payload.
        /// @return The attributes, or nothing when malformed.
        static std::optional<OwnAttributes> decode(const core::Bytes& payload);
    };

}

#endif
