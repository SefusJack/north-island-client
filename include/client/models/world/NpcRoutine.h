#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_NPC_ROUTINE_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_NPC_ROUTINE_H

#include "core/Point.h"
#include "core/Rect.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace client {

    /// @brief How a map NPC moves on its own: stands, walks the points of its path, or wanders to random points of an area.
    /// @details Original: TMapNpc.UpdateMove. It waits at each point its time; at the end of a path it stops, starts again or walks
    /// back; a wanderer waits the same time at each point. Walks between points are straight lines.
    struct NpcRoutine {
        /// @brief The kind of routine.
        enum class Kind { Stand, Patrol, Wander };

        /// @brief What a patrol does at the end of its path.
        enum class End { Stop, Again, Back };

        Kind kind = Kind::Stand;
        End end = End::Stop;
        std::vector<core::Point> points;
        std::vector<int> waits;
        core::Rect area;
        int wanderWaitMs = 0;
        std::size_t index = 0;
        std::uint64_t arrivedMs = 0;
    };

}

#endif
