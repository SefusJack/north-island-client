#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_THROW_TEAM_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_THROW_TEAM_H

#include "client/subsystems/minigames/ThrowThrower.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief One side of the throwing game: its people, their states, hearts and AI bookkeeping.
    /// @details States: 0 standing, 1 charging or winding up, 2 sent to walk, 3 hit, 5 walking, 6 about to throw.
    struct ThrowTeam {
        static constexpr int kMost = 4;

        std::array<ThrowThrower, kMost + 1> throwers;
        std::array<int, kMost + 1> states{};
        std::array<int, kMost + 1> lives{};
        std::array<int, kMost + 1> modes{};
        std::array<int, kMost + 1> aiDelays{};
        std::array<std::uint64_t, kMost + 1> decisionMs{};
        std::array<std::uint64_t, kMost + 1> throwMs{};
        std::array<std::uint64_t, kMost + 1> hitMs{};
        std::array<std::uint64_t, kMost + 1> moveTickX{};
        std::array<std::uint64_t, kMost + 1> moveTickY{};
        std::array<int, kMost + 1> distX{};
        std::array<int, kMost + 1> distY{};
        int count = 0;
    };

}

#endif
