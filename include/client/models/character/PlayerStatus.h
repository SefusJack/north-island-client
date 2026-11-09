#ifndef NORTH_ISLAND_CLIENT_MODELS_CHARACTER_PLAYER_STATUS_H
#define NORTH_ISLAND_CLIENT_MODELS_CHARACTER_PLAYER_STATUS_H

#include "protocol/LearnedSkill.h"

#include <array>
#include <cstdint>
#include <map>

namespace client {

    /// @brief What the HUD shows of the own player.
    /// @details Fed by server 5/3, 8/1 and 26/x.
    struct PlayerStatus {
        int element = 0;
        std::int32_t hp = 0;
        std::int32_t maxHp = 0;
        int sp = 0;
        int maxSp = 0;
        // 5 base stats
        std::array<int, 5> stats{};
        // 5 combat values
        std::array<int, 5> combat{};
        int potential = 0;
        int level = 0;
        std::int64_t exp = 0;
        int reborn = 0;
        int job = 0;
        int skillPoints = 0;
        int statPoints = 0;
        std::int64_t gold = 0;
        std::int32_t hpBottle = 0;
        std::int32_t spBottle = 0;
        std::map<int, protocol::LearnedSkill> skills;
    };

}

#endif
