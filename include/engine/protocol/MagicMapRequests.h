#ifndef NORTH_ISLAND_PROTOCOL_MAGIC_MAP_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_MAGIC_MAP_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 23/89: teleport to a place on the teleport items' map.
    /// @param place Place on the map (u8, 1..40).
    /// @param bagSlot Bag place of the item (u8).
    /// @param mode 1 the Teleport Scroll, 2 the Bamboo Dragonflies.
    /// @return The encoded payload.
    core::Bytes magicMapTeleportRequest(int place, int bagSlot, int mode);

}

#endif
