#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_PLAYER_TRAILS_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_PLAYER_TRAILS_H

#include "client/models/world/FollowTrail.h"
#include "core/Point.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <random>

namespace client {

    /// @brief Where the players on the map stand and the trails behind them, which their pets and team members follow.
    /// @details Original: TBasePlayer.FTrailPoints.
    class PlayerTrails {
        public:
            /// @brief Records where a player stands now.
            /// @param playerId Player id.
            /// @param position Map position.
            /// @param placed True when put there (entering a map, jumping), so the trail starts anew.
            void at(std::int32_t playerId, core::Point position, bool placed);

            /// @brief Puts a player on a map with the trail points scattered around the place.
            /// @details Original: a player put on a map by 3/12 (ScatterPetsAround).
            /// @param playerId Player id.
            /// @param position Map position.
            /// @param random Random generator.
            void scatter(std::int32_t playerId, core::Point position, std::mt19937& random);

            /// @brief Forgets a player.
            /// @param playerId Player id.
            void remove(std::int32_t playerId) { m_players.erase(playerId); }

            /// @brief Tells whether a player is known.
            /// @param playerId Player id.
            /// @return True when known.
            bool knows(std::int32_t playerId) const { return m_players.count(playerId) != 0; }

            /// @brief Gives where a player stands (0,0 for one not seen yet).
            /// @param playerId Player id.
            /// @return The map position.
            core::Point position(std::int32_t playerId) const;

            /// @brief Gives a point of a player's trail.
            /// @param playerId Player id.
            /// @param index Trail point index, 0 is the newest.
            /// @return The point (0,0 for an unknown player).
            core::Point point(std::int32_t playerId, std::size_t index) const;

        private:
            /// @brief One player's position and trail.
            struct Player {
                core::Point position;
                FollowTrail trail;
            };

            std::map<std::int32_t, Player> m_players;
    };

}

#endif
