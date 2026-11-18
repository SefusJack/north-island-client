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
        bool sticky = false; // it follows the mouse with no button held, until the next left click puts it down
        int count = 0; // how many of the source's things it carries (0: as the source decides)
        bool fromClick = false; // a click took it up (the modern way): a double click there gives it back and goes on as a double click
    };

}

#endif
