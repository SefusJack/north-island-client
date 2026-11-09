#ifndef NORTH_ISLAND_CLIENT_RULES_NPC_MOVEMENT_H
#define NORTH_ISLAND_CLIENT_RULES_NPC_MOVEMENT_H

#include "client/models/world/MapNpc.h"
#include "client/rules/Walking.h"

#include <cstdint>
#include <random>

namespace client {

    /// @brief Moves a map NPC on: along its walk, or, once it has waited at a point, off to the next one of its routine.
    /// @param npc The NPC (one an event holds stands).
    /// @param nowMs Current time in ms.
    /// @param random Random source for wandering NPCs.
    /// @param steps How many walk steps to take.
    void moveNpc(MapNpc& npc, std::uint64_t nowMs, std::mt19937& random, WalkSteps steps = WalkSteps::Whole);

}

#endif
