#ifndef NORTH_ISLAND_PROTOCOL_CHARACTER_SUMMARY_H
#define NORTH_ISLAND_PROTOCOL_CHARACTER_SUMMARY_H

#include <array>
#include <cstdint>
#include <string>

namespace protocol {

    /// @brief One character of the account as the character list (server 63/1) describes it.
    struct CharacterSummary {
        int slot = 0;
        std::string name;
        int level = 0;
        int element = 0;
        std::int32_t maxHp = 0;
        std::int32_t hp = 0;
        std::int32_t maxSp = 0;
        std::int32_t sp = 0;
        std::int32_t exp = 0;
        std::int32_t gold = 0;
        int bodyType = 0;
        int appearance = 0;
        int head = 0;
        int spriteId = 0;
        std::int32_t colorPattern1 = 0;
        std::int32_t colorPattern2 = 0;
        int reborn = 0;
        int job = 0;
        std::array<std::uint16_t, 6> equipment{};
    };

}

#endif
