#ifndef NORTH_ISLAND_FORMATS_MAP_TILE_H
#define NORTH_ISLAND_FORMATS_MAP_TILE_H

#include <cstdint>

namespace formats {

    /// @brief One tile of a map built from isometric tiles.
    /// @details Its ground picture is "<ground><A..F>" (none when the variant is 0) and the object standing on it
    /// "<objectBase + objectNumber><A..D>" (none when the base is 0), with the map row the object stands on.
    struct MapTile {
        int ground = 0;
        int objectBase = 0;
        int groundVariant = 0;
        int objectNumber = 0;
        int objectVariant = 0;
        std::int32_t sortKey = 0;
    };

}

#endif
