#ifndef NORTH_ISLAND_CLIENT_RENDERING_HUD_PAGE_NUMBERS_H
#define NORTH_ISLAND_CLIENT_RENDERING_HUD_PAGE_NUMBERS_H

#include "core/Point.h"
#include "gfx/Canvas.h"

namespace client {

    /// @brief Draws "page / pages" from Num_MainInfo: the slash at the point, the page ending just before it and the page count after it.
    /// @details Original: the layout the game uses for its later data (JmaLevel above 0). Num_MainInfo has 11 glyph rows; the slash is the twelfth.
    /// @param canvas Target canvas.
    /// @param slash Position of the slash.
    /// @param page Current page number.
    /// @param pages Number of pages.
    void drawPageNumbers(gfx::Canvas& canvas, core::Point slash, int page, int pages);

    /// @brief Draws "page / pages" as the older forms (the instance windows) lay it out: the page number starts at the point, one glyph further left when it has two digits.
    /// @param canvas Target canvas.
    /// @param start Position of the page number.
    /// @param page Current page number.
    /// @param pages Number of pages.
    void drawPageNumbersFrom(gfx::Canvas& canvas, core::Point start, int page, int pages);

}

#endif
