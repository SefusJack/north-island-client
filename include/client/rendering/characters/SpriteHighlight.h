#ifndef NORTH_ISLAND_CLIENT_RENDERING_CHARACTERS_SPRITE_HIGHLIGHT_H
#define NORTH_ISLAND_CLIENT_RENDERING_CHARACTERS_SPRITE_HIGHLIGHT_H

#include <cstdint>

namespace client {

    /// @brief How a sprite under the mouse stands out, and how it is scaled, tinted or blended.
    /// @details Original: DrawMovieRoleSprite. Added once more at light / 255; in a battle (marked) its palette entry 255, otherwise the
    /// transparent green, is drawn yellow (255, 255, 10). A ghost (draw mode 3, the fly-away's afterimages) is only the added pass.
    /// A status colour (TintPaletteByKind, kinds 1..8) shifts the palette's entries 16..223. In battle a sprite may be drawn larger or
    /// smaller (DrawScale / ScaleDiv around its feet), the layers that sit lower (large NPC sprites, weapons) dropping a little more.
    /// A translucent sprite shows the ground through it.
    struct SpriteHighlight {
        std::uint8_t light = 0;
        bool marked = false;
        bool ghost = false;
        int statusTint = 0;
        // scale / scaleDivisor of 10 / 10 is the normal size
        int scale = 10;
        int scaleDivisor = 10;
        int lowDrop = 0;
        // 255 is fully opaque
        std::uint8_t opacity = 255;
        bool mirrored = false;
    };

}

#endif
