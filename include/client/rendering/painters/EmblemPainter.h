#ifndef NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_EMBLEM_PAINTER_H
#define NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_EMBLEM_PAINTER_H

#include "core/Point.h"
#include "formats/EmblemBlock.h"
#include "gfx/Canvas.h"

namespace client {

    /// @brief Paints a badge's pixels in the colours of the emblem palette, the ones without a colour of their own left as they are.
    /// @details Original: DrawEmblemOnSurface and DrawEmblemToBits.
    struct EmblemPainter {
        /// @brief Draws the badge pixel by pixel; the pixel of column x and row y is number x * 24 + y.
        /// @param canvas Target canvas.
        /// @param badge The badge's pixels.
        /// @param at Top left corner on the canvas.
        static void paint(gfx::Canvas& canvas, const formats::EmblemBlock& badge, core::Point at);
    };

}

#endif
