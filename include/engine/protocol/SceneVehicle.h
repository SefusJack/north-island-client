#ifndef NORTH_ISLAND_PROTOCOL_SCENE_VEHICLE_H
#define NORTH_ISLAND_PROTOCOL_SCENE_VEHICLE_H

#include <cstdint>

namespace protocol {

    /// @brief A vehicle standing on the map: list slot, owner, item, position and the owner's bag slot it came from.
    /// @details Original: a record of server 23/102, HandleSceneVehicleListPacket.
    struct SceneVehicle {
        int slot = 0;
        std::int32_t ownerId = 0;
        int itemId = 0;
        std::int32_t x = 0;
        std::int32_t y = 0;
        int bagSlot = 0;
    };

}

#endif
