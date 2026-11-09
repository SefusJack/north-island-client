#ifndef NORTH_ISLAND_PROTOCOL_SCENE_RIDER_H
#define NORTH_ISLAND_PROTOCOL_SCENE_RIDER_H

#include <cstdint>

namespace protocol {

    /// @brief A player riding a vehicle.
    /// @details Original: a record of server 23/115, HandleRiderListPacket.
    struct SceneRider {
        std::int32_t playerId = 0;
        int itemId = 0;
    };

}

#endif
