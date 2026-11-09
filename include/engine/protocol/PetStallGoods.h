#ifndef NORTH_ISLAND_PROTOCOL_PET_STALL_GOODS_H
#define NORTH_ISLAND_PROTOCOL_PET_STALL_GOODS_H

#include "protocol/PetInfoBlock.h"

#include <cstdint>
#include <vector>

namespace protocol {

    /// @brief What someone's pet stall sells: the owner and up to three pets with prices, numbers, gear and info block.
    /// @details Original: server 23/113 (RecvPetStallOffer).
    struct PetStallGoods {
        /// @brief One pet on sale.
        struct Pet {
            int stallSlot = 0;
            int npcId = 0;
            std::int32_t price = 0;
            int level = 0;
            std::int32_t hp = 0;
            std::int32_t maxHp = 0;
            int maxSp = 0;
            std::vector<int> equipment;
            PetInfoBlock info;
        };

        std::int32_t ownerId = 0;
        std::vector<Pet> pets;
    };

}

#endif
