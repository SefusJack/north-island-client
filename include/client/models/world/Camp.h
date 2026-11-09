#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_CAMP_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_CAMP_H

#include "core/Point.h"

#include <cstdint>
#include <string>

namespace client {

    /// @brief A tent on the map: owner, tent item, position and style.
    /// @details Original: TRe_Camp. Style is kind 1 of the Tent Palette's set or kind 2 of the Shape Clay's.
    struct Camp {
        std::int32_t ownerId = 0;
        int tentItemId = 0;
        core::Point position;
        int kind1 = 0;
        int kind2 = 0;
        int frameHeight = 0;
        bool own = false;
        std::string ownerName;
        std::string ownerTitle;
    };

}

#endif
