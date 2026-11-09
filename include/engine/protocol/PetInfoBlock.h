#ifndef NORTH_ISLAND_PROTOCOL_PET_INFO_BLOCK_H
#define NORTH_ISLAND_PROTOCOL_PET_INFO_BLOCK_H

#include <cstdint>

namespace protocol {

    /// @brief The 86-byte pet record that a pet trade offer (25/12) and a pet stall (23/113) carry.
    /// @details Original: TPetBlk_718; the kind, HP, SP, the five attributes, level and amity.
    struct PetInfoBlock {
        int npcId = 0;
        std::int32_t hp = 0;
        int sp = 0;
        int strength = 0;
        int constitution = 0;
        int intelligence = 0;
        int wisdom = 0;
        int agility = 0;
        int level = 0;
        int amity = 0;
    };

}

#endif
