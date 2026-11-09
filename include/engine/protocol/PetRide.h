#ifndef NORTH_ISLAND_PROTOCOL_PET_RIDE_H
#define NORTH_ISLAND_PROTOCOL_PET_RIDE_H

#include <cstdint>

namespace protocol {

    /// @brief One player riding a pet of the scene (server 23/116, HandleRidePetListPacket).
    struct PetRide {
        int place = 0;
        std::int32_t playerId = 0;
        int npcId = 0;
    };

}

#endif
