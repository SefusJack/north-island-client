#ifndef NORTH_ISLAND_CLIENT_MODELS_SOCIAL_MAGNET_PLAYER_H
#define NORTH_ISLAND_CLIENT_MODELS_SOCIAL_MAGNET_PLAYER_H

#include <cstdint>
#include <string>

namespace client {

    /// @brief What the Magic Magnet's checks read of a player the world knows.
    /// @details Original: WorldPlayers[n]: FPlayerId, Name, FLevel, FReborn and the Magic Magnet worn in FEquips[6].
    struct MagnetPlayer {
        std::int32_t id = 0;
        std::string name;
        int level = 0;
        int reborn = 0;
        bool wearsMagnet = false;
    };

}

#endif
