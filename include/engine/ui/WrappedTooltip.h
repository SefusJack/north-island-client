#ifndef NORTH_ISLAND_UI_WRAPPED_TOOLTIP_H
#define NORTH_ISLAND_UI_WRAPPED_TOOLTIP_H

#include "core/Color.h"
#include "core/Point.h"
#include "gfx/Canvas.h"

#include <string>

namespace ui {

    /// @brief Draws a tooltip box with the text wrapped into lines of lineChars characters.
    /// @details Original: DrawWrappedTooltip. Uses WrapTextLines; the box has its top left at the given point, is moved back
    /// onto the screen (and 30 pixels clear of its bottom), filled see-through and framed, the text drawn with a shadow.
    /// @param canvas Target canvas.
    /// @param at Top left of the box.
    /// @param text Tooltip text.
    /// @param lineChars Characters per line.
    /// @param lineHeight Height of a text line in pixels.
    /// @param textColor Text colour.
    /// @param background Box fill colour.
    /// @param border Box outline colour.
    void drawWrappedTooltip(
        gfx::Canvas& canvas,
        core::Point at,
        const std::string& text,
        int lineChars,
        int lineHeight,
        core::Color textColor,
        core::Color background,
        core::Color border
    );

}

#endif
