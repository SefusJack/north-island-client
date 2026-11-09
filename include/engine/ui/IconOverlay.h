#ifndef NORTH_ISLAND_UI_ICON_OVERLAY_H
#define NORTH_ISLAND_UI_ICON_OVERLAY_H

#include "core/Point.h"
#include "core/Rect.h"
#include "gfx/PictureId.h"

namespace ui {

    /// @brief A picture part drawn over an IconGrid cell's icon at an offset from the cell's top left.
    struct IconOverlay {
        gfx::PictureId picture;
        core::Rect source;
        core::Point offset;
    };

}

#endif
