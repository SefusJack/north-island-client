#ifndef NORTH_ISLAND_UI_ICON_CELL_H
#define NORTH_ISLAND_UI_ICON_CELL_H

#include "core/Rect.h"
#include "core/Size.h"
#include "gfx/PictureId.h"
#include "ui/IconOverlay.h"

#include <optional>
#include <string>
#include <vector>

namespace ui {

    /// @brief What one cell of an IconGrid shows: part of a picture (optionally resized), an optional count and hint, and overlays.
    struct IconCell {
        gfx::PictureId picture;
        core::Rect source;
        gfx::PictureId frame;
        std::optional<int> count;
        std::string hint;
        std::vector<IconOverlay> overlays;
        std::optional<core::Size> size; // the source drawn shrunk or stretched to this size, centred in the cell; as it is when none
    };

}

#endif
