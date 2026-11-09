#ifndef NORTH_ISLAND_CLIENT_RENDERING_CHARACTERS_FIGURE_SHADOW_H
#define NORTH_ISLAND_CLIENT_RENDERING_CHARACTERS_FIGURE_SHADOW_H

#include "core/Point.h"
#include "gfx/Canvas.h"

namespace client {

    /// @brief Draws the shadow under a figure's feet.
    /// @details Kind 0 is the small dithered one of people (part of "Shadow"), 1 the large monster shadow, anything else none
    /// (Npc.dat's shadow column).
    /// @param canvas Target canvas.
    /// @param feet Screen position of the feet.
    /// @param kind Shadow kind from Npc.dat.
    void drawFigureShadow(gfx::Canvas& canvas, core::Point feet, int kind);

}

#endif
