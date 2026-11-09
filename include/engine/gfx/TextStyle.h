#ifndef NORTH_ISLAND_GFX_TEXT_STYLE_H
#define NORTH_ISLAND_GFX_TEXT_STYLE_H

#include "core/Color.h"

#include <optional>

namespace gfx {

    /// @brief The game's three text looks: Plain (glyphs only), Outline (edge colour right and below, a dark shadow two pixels
    /// further, one pixel more advance per character) and Shadow (a dark shadow one pixel right and below).
    enum class TextEffect { Plain, Outline, Shadow };

    /// @brief How a text is drawn: colour, effect and optional cell background.
    struct TextStyle {
        core::Color color{255, 255, 255, 255}; // opaque white
        TextEffect effect = TextEffect::Plain;
        std::optional<core::Color> background;
    };

}

#endif
