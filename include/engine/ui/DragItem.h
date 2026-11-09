#ifndef NORTH_ISLAND_UI_DRAG_ITEM_H
#define NORTH_ISLAND_UI_DRAG_ITEM_H

#include "core/Point.h"
#include "core/Rect.h"
#include "gfx/PictureId.h"

namespace ui {

    /// @brief What the mouse carries while an icon is dragged: the source element and cell, and the picture drawn where the mouse holds it.
    /// @details Original: Se_DragItem.
    struct DragItem {
        int sourceId = 0;
        int sourceIndex = 0;
        gfx::PictureId picture;
        core::Rect source;
        core::Point grab;
    };

}

#endif
