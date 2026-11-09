#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_RIDDEN_PET_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_RIDDEN_PET_H

#include "formats/NpcInfo.h"

namespace client {

    /// @brief The pet a player rides, drawn under the rider, who sits higher by the pet's height.
    /// @details Original: FRidePetNpc, FRidePetSlot, FRideHeight.
    struct RiddenPet {
        int place = 0;
        formats::NpcInfo info;
        int height = 0;
    };

}

#endif
