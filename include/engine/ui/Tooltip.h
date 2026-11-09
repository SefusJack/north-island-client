#ifndef NORTH_ISLAND_UI_TOOLTIP_H
#define NORTH_ISLAND_UI_TOOLTIP_H

#include "core/Point.h"
#include "gfx/Canvas.h"

#include <string>

namespace ui {

    /// @brief Draws a tooltip box ending at the anchor, in rows of 40 characters, kept on the screen, with white text.
    /// @details Original: DrawTooltipAt.
    /// @param canvas Target canvas.
    /// @param anchor Bottom right corner of the box.
    /// @param text Tooltip text.
    void drawTooltipAt(gfx::Canvas& canvas, core::Point anchor, const std::string& text);

    /// @brief Draws the hint box of a hovered element above a point, in lines of 50 characters, kept in the window, with white text.
    /// @param canvas Target canvas.
    /// @param origin The point the box sits above (the element's hint origin).
    /// @param text Hint text.
    void drawHintBox(gfx::Canvas& canvas, core::Point origin, const std::string& text);

}

#endif
