#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_STALL_IN_FUR_ITEM_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_STALL_IN_FUR_ITEM_H

#include "core/Point.h"
#include "core/Size.h"
#include "gfx/PictureId.h"
#include "protocol/ItemInstance.h"

#include <cstdint>

namespace client {

    /// @brief An item on sale on a shelf or in a vending machine of a house.
    /// @details Original: DZ_Thing.pas TStallInFurItem; item id 0 is a free place.
    struct StallInFurItem {
        int itemId = 0;
        int furnIndex = 0;
        int posInFurn = 0;
        int stallIndex = 0;
        int count = 0;
        std::int32_t price = 0;
        int gridSlot = 0;
        core::Point world;
        protocol::ItemInstance item;
        int echoItem = 0;
        std::int32_t echoPrice = 0;
        gfx::PictureId picture;
        core::Size size;
    };

}

#endif
