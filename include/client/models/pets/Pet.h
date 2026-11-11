#ifndef NORTH_ISLAND_CLIENT_MODELS_PETS_PET_H
#define NORTH_ISLAND_CLIENT_MODELS_PETS_PET_H

#include "formats/NpcInfo.h"
#include "protocol/ItemInstance.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace client {

    /// @brief One of the own player's pets (original TFollowNpc): kind from Npc.dat, name, join order (1..4) and state.
    struct Pet {
        formats::NpcInfo info;
        std::string name;
        int order = 0;
        int level = 1;
        int reborn = 0;
        std::int32_t exp = 0;
        std::int32_t hp = 0;
        std::int32_t maxHp = 0;
        int sp = 0;
        int maxSp = 0;
        int strength = 0;
        int constitution = 0;
        int intelligence = 0;
        int wisdom = 0;
        int agility = 0;
        int maxHpBonus = 0;
        int maxSpBonus = 0;
        std::array<int, 5> combatBonus{};
        std::array<int, 5> recalcStats{};
        std::array<int, 5> recalcBonus{};
        std::array<int, 5> serverCombat{};
        bool serverCombatKnown = false; // true while serverCombat matches the pet's current stats and bonuses
        int amity = 0;
        int tradable = 0;
        int potential = 0;
        int training = 0;
        std::array<int, 3> skillLevels{};
        std::array<int, 3> skillExp{};
        std::array<protocol::ItemInstance, 7> worn{};

        /// @brief Lists the item ids the pet wears.
        /// @return The ids of the worn items, in slot order.
        std::vector<int> equipmentIds() const;
    };

}

#endif
