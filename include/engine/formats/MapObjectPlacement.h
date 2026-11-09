#ifndef NORTH_ISLAND_FORMATS_MAP_OBJECT_PLACEMENT_H
#define NORTH_ISLAND_FORMATS_MAP_OBJECT_PLACEMENT_H

#include "core/Point.h"

#include <cstdint>

namespace formats {

    /// @brief A ground object placed on a map.
    /// @details Original: which object (Wem.MMG "<objectId>.wem") and the map pixel where its picture's top-left corner goes.
    struct MapObjectPlacement {
        std::uint32_t objectId = 0;
        core::Point position;
    };

}

#endif
