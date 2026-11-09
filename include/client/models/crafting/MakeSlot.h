#ifndef NORTH_ISLAND_CLIENT_MODELS_CRAFTING_MAKE_SLOT_H
#define NORTH_ISLAND_CLIENT_MODELS_CRAFTING_MAKE_SLOT_H

#include <cstdint>

namespace client {

    /// @brief One of the five things the player has under way: what is made, how long it has been worked on, in how many batches, and whether it is being worked on now.
    /// @details Original: DZ_Thing.pas TUtensilManage.FMakeSlots.
    struct MakeSlot {
        int productId = 0;
        std::uint32_t elapsedMs = 0;
        int batches = 0;
        bool active = false;
    };

}

#endif
