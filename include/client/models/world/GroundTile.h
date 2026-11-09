#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_GROUND_TILE_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_GROUND_TILE_H

#include "core/Point.h"
#include "gfx/PictureId.h"

namespace client {

    /// @brief A tile of an isometric-tile map, ready to draw.
    /// @details Original: TMap.DrawMapTiles, DrawIsoTileObject. Holds where its ground picture goes (map pixels), the picture of the
    /// object standing on it and where that goes, the extra back part some objects have, and the map row the object stands on (for
    /// sorting with the people). Its grid place and the numbers its pictures were named after stay with it (the small map's rules look at them).
    struct GroundTile {
        int row = 0;
        int column = 0;
        int groundNumber = 0;
        int objectBase = 0;
        gfx::PictureId ground;
        core::Point groundPosition;
        gfx::PictureId object;
        core::Point objectPosition;
        gfx::PictureId objectBack;
        core::Point objectBackPosition;
        int sortY = 0;
    };

}

#endif
