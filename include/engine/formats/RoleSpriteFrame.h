#ifndef NORTH_ISLAND_FORMATS_ROLE_SPRITE_FRAME_H
#define NORTH_ISLAND_FORMATS_ROLE_SPRITE_FRAME_H

#include <cstdint>
#include <vector>

namespace formats {

    /// @brief One frame of a role sprite: an 8-bit paletted image cropped out of a larger canvas.
    /// @details The canvas is anchored at the character's feet (see RoleSprite).
    struct RoleSpriteFrame {
        int canvasWidth = 0;
        int canvasHeight = 0;
        int offsetX = 0;
        int offsetY = 0;
        int width = 0;
        int height = 0;
        std::vector<std::uint8_t> pixels;
    };

}

#endif
