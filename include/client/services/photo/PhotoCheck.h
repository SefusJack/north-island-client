#ifndef NORTH_ISLAND_CLIENT_SERVICES_PHOTO_PHOTO_CHECK_H
#define NORTH_ISLAND_CLIENT_SERVICES_PHOTO_PHOTO_CHECK_H

#include "core/Point.h"

#include <cstdint>
#include <vector>

namespace client {

    /// @brief What the .bft check file of a photo says of the world it was taken in: the player, its map and place, and the other players of the scene with theirs.
    /// @details Original: TForm1.GetPicAndSave2 into ft_CheckBmp.
    struct PhotoCheck {
        /// @brief Another player of the scene and where it stood.
        struct Player {
            std::int32_t id = 0;
            core::Point position;
        };

        std::int32_t playerId = 0;
        int mapId = 0;
        core::Point position;
        std::vector<Player> players;
    };

}

#endif
