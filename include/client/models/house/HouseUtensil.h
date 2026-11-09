#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_UTENSIL_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_UTENSIL_H

#include "core/Point.h"
#include "gfx/PictureId.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief A piece of furniture standing in a room as the game draws it: its facing picture, floor point on the map and animation.
    /// @details Original: DZ_Thing.pas TGdUtensil. Some pieces carry a little animation (a fire, a clock, a cracker, a lift).
    struct HouseUtensil {
        int itemId = 0;
        bool shown = true;
        int furnIndex = 0;
        std::array<int, 11> contentSlots{}; // places 1..10 of a piece, index 0 unused
        std::int32_t salePrice = 0;
        int eggCount = 0;
        core::Point world;
        gfx::PictureId picture;
        gfx::PictureId overlayPicture;
        gfx::PictureId animPicture;
        core::Point animOffset;
        core::Point animCell;
        int animFrames = 0;
        int animWidth = 0;
        int animHeight = 0;
        int animFrame = 0;
        int animState = 0;
        std::uint64_t animMs = 0;
    };

}

#endif
