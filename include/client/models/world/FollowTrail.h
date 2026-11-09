#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_FOLLOW_TRAIL_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_FOLLOW_TRAIL_H

#include "core/Point.h"

#include <array>
#include <cstddef>
#include <random>

namespace client {

    /// @brief Where a player has been: five points, the newest first, that a fighting pet follows.
    /// @details Original: TBasePlayer.FTrailPoints. A new point is pushed whenever the player is 20 pixels or more (across or down)
    /// from the newest. The fighting pet heads for the third (FollowActivePet); the points lie further apart the longer a frame takes.
    class FollowTrail {
        public:
            static constexpr std::size_t kPetPoint = 2;

            /// @brief Puts every point on one place.
            /// @param at The place.
            void reset(core::Point at) { m_points.fill(at); }

            /// @brief Scatters each point a random 0..29 pixels right of and below a place.
            /// @details Original: ScatterPetsAround (FTrailPoints[I] := Random(30) + X, Random(30) + Y).
            /// @param at The place.
            /// @param random Random generator.
            void scatter(core::Point at, std::mt19937& random);

            /// @brief Pushes a new point when the player moved 20 pixels or more from the newest.
            /// @details Original: TBasePlayer.UpdateMove.
            /// @param at The player's position.
            void update(core::Point at);

            /// @brief Gets a trail point.
            /// @param index Point index, 0 is the newest.
            /// @return The point.
            core::Point at(std::size_t index) const { return m_points[index]; }

        private:
            std::array<core::Point, 5> m_points{}; // the trail keeps five points
    };

}

#endif
