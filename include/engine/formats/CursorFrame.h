#ifndef NORTH_ISLAND_FORMATS_CURSOR_FRAME_H
#define NORTH_ISLAND_FORMATS_CURSOR_FRAME_H

#include "core/Point.h"
#include "formats/Image.h"

namespace formats {

    /// @brief One picture of a mouse cursor and the pixel that points.
    struct CursorFrame {
        Image image;
        core::Point hotspot;
    };

}

#endif
