#ifndef NORTH_ISLAND_CLIENT_RENDERING_HUD_INFO_DIGITS_H
#define NORTH_ISLAND_CLIENT_RENDERING_HUD_INFO_DIGITS_H

#include "core/Point.h"
#include "gfx/Canvas.h"

namespace client {

    /// @brief Draws a number in the digits of Num_MainInfo (11 glyph rows), one glyph's width apart from the point on.
    /// @param canvas Target canvas.
    /// @param at Position of the first digit.
    /// @param value The number.
    void drawInfoDigits(gfx::Canvas& canvas, core::Point at, int value);

}

#endif
