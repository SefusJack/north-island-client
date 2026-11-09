#ifndef NORTH_ISLAND_FORMATS_NPC_INFO_H
#define NORTH_ISLAND_FORMATS_NPC_INFO_H

#include <array>
#include <cstdint>
#include <string>

namespace formats {

    /// @brief One NPC kind of Npc.dat.
    /// @details Its name, type (4 people, 7 monsters, 6 / 9 / 10 scene objects), sprite (1xxx in the 001 banks) and how the sprite is drawn.
    /// Pets and fighters use the role sprite (7xxx in the 007 banks) in the NPC's four colour codes.
    struct NpcInfo {
        int npcId = 0;
        std::string name;
        int type = 0;
        int spriteId = 0;
        int roleSpriteId = 0;
        std::array<std::int32_t, 4> colors{};
        int uncatchable = 0;
        int bodyType = 0;
        int weaponType = 0;
        int level = 0;
        std::int32_t barHp = 0;
        int heightMode = 0;
        int element = 0;
        std::array<int, 3> skills{};
        int shadow = 0;
        int directionKind = 0;
        int spriteSize = 0;
        int fixedSkill = 0;
        int faceSpriteId = 0;
        int weapon = 0;
        int noPk = 0;
        int growKey = 0;
        int multiHpBars = 0;
        int mercenary = 0;
        int npcKey = 0;
        int stateSoundNo = 0;
        std::int32_t collectionNumber = 0;
        int strength = 0;
        int constitution = 0;
        int intelligence = 0;
        int wisdom = 0;
        int agility = 0;
    };

}

#endif
