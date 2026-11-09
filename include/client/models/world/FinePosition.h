#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_FINE_POSITION_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_FINE_POSITION_H

#include "core/Point.h"

namespace client {

    /// @brief Where a walker really is between whole pixels, and the whole pixel made of it.
    /// @details Original: WalkSteps::SubPixel. When the character's position is no longer that pixel, the fraction is dropped.
    struct FinePosition {
        double x = 0;
        double y = 0;
        core::Point rounded;
    };

}

#endif
