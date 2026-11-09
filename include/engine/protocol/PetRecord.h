#ifndef NORTH_ISLAND_PROTOCOL_PET_RECORD_H
#define NORTH_ISLAND_PROTOCOL_PET_RECORD_H

#include "protocol/ItemInstance.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace protocol {

    /// @brief One of the own player's pets as the server sends it, from the pet list (15/8) or a pet that joined (15/1).
    /// @details Original: TFollowNpc. A pet is known by its Npc.dat id; a player has one of a kind.
    struct PetRecord {
        int slot = 0;
        int npcId = 0;
        std::string name;
        int level = 0;
        std::int32_t exp = 0;
        std::int32_t hp = 0;
        int sp = 0;
        int strength = 0;
        int constitution = 0;
        int intelligence = 0;
        int wisdom = 0;
        int agility = 0;
        int amity = 0;
        int tradable = 0;
        int reborn = 0;
        int potential = 0;
        int training = 0;
        std::array<int, 3> skillLevels{};
        std::array<int, 3> skillExp{};
        std::vector<int> equipment;
        std::vector<ItemInstance> worn;
    };

}

#endif
