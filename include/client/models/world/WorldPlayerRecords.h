#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_WORLD_PLAYER_RECORDS_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_WORLD_PLAYER_RECORDS_H

#include "protocol/OtherPlayerRecord.h"

#include <cstdint>
#include <functional>
#include <map>
#include <optional>

namespace client {

    using WorldPlayerRecords = std::map<std::int32_t, protocol::OtherPlayerRecord>;

    /// @brief Moves a player's record to the map and place the server names.
    /// @details Original: SetWorldPlayerMap (S->C 12 for another player's id).
    /// @param records The world player list.
    /// @param playerId Player id.
    /// @param mapId The map id.
    /// @param x Place x.
    /// @param y Place y.
    /// @return The updated record, or nothing when the list does not know the player.
    std::optional<std::reference_wrapper<const protocol::OtherPlayerRecord>> setWorldPlayerMap(
        WorldPlayerRecords& records,
        std::int32_t playerId,
        int mapId,
        std::uint16_t x,
        std::uint16_t y
    );

}

#endif
