#ifndef NORTH_ISLAND_UI_DRAW_CONTEXT_H
#define NORTH_ISLAND_UI_DRAW_CONTEXT_H

#include "core/Point.h"
#include "gfx/Canvas.h"
#include "ui/DragItem.h"

#include <cstdint>
#include <map>
#include <utility>

namespace ui {

    /// @brief What a grid cell is about to show while things move the modern way: a carried part has left it, or a move the
    /// server has not answered yet has filled it.
    struct CellPreview {
        gfx::PictureId picture;
        core::Rect source;
        int count = 0; // how many it shows; 0 or less: it shows nothing
    };

    /// @brief State passed down the element tree while drawing: canvas, hovered and focused ids, mouse and time.
    struct DrawContext {
        gfx::Canvas& canvas;
        int hoveredId = 0;
        int focusedId = 0;
        core::Point mouse;
        std::uint64_t nowMs = 0;
        const std::map<std::pair<int, int>, CellPreview>* previews = nullptr; // (grid id, cell): what the cell is about to show
    };

}

#endif
