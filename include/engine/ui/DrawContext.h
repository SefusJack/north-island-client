#ifndef NORTH_ISLAND_UI_DRAW_CONTEXT_H
#define NORTH_ISLAND_UI_DRAW_CONTEXT_H

#include "core/Point.h"
#include "gfx/Canvas.h"

#include <cstdint>

namespace ui {

    /// @brief State passed down the element tree while drawing: canvas, hovered and focused ids, mouse and time.
    struct DrawContext {
        gfx::Canvas& canvas;
        int hoveredId = 0;
        int focusedId = 0;
        core::Point mouse;
        std::uint64_t nowMs = 0;
    };

}

#endif
