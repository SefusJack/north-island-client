#ifndef NORTH_ISLAND_PROTOCOL_PET_JOINED_H
#define NORTH_ISLAND_PROTOCOL_PET_JOINED_H

#include "protocol/PetRecord.h"

#include <cstdint>

namespace protocol {

    /// @brief A pet joined a player's team (the own one, or another's first place); server 15/1.
    struct PetJoined {
        std::int32_t ownerId = 0;
        PetRecord pet;
    };

}

#endif
