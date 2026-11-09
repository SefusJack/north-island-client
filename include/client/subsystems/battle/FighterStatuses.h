#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_FIGHTER_STATUSES_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_FIGHTER_STATUSES_H

#include "client/subsystems/battle/StatusSlot.h"

#include <array>
#include <cstdint>
#include <vector>

namespace client {

    /// @brief The statuses on a fighter and what they change in the figure.
    /// @details The main ones by their kind (1, 2, 3, 4 and 8, as 53/1 numbers them), the eight lesser ones (FSubStatus,
    /// kind 5 with their values), the skills shown as icons in the battle panel (FStatusSlots: up to five, and the two
    /// special ones of kinds 6 and 7), and the figure's size (FSizeMode 1 bigger, 2 smaller) and colour (FTintKind, from the
    /// skill FStatusSkillId).
    struct FighterStatuses {
        static constexpr int kMostIcons = 5;

        std::array<StatusSlot, 9> main{}; // indexed by status kind 1..8, index 0 unused
        std::array<StatusSlot, 9> sub{}; // the eight lesser statuses 1..8, index 0 unused
        std::array<std::int32_t, 9> values{}; // values of the lesser statuses 1..8, index 0 unused
        std::vector<int> icons;
        std::array<int, 3> special{}; // the special statuses of kinds 6 and 7, index 0 unused
        int sizeMode = 0;
        bool sub53 = false;
        int tintKind = 0;
        int tintSkill = 0;
    };

}

#endif
