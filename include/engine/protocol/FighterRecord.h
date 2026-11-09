#ifndef NORTH_ISLAND_PROTOCOL_FIGHTER_RECORD_H
#define NORTH_ISLAND_PROTOCOL_FIGHTER_RECORD_H

#include <cstdint>

namespace protocol {

    /// @brief A fighter of a battle (server 11/250, 30 bytes a record).
    /// @details Layout: u8 state, u8 kind, i32 id, u16 map NPC, i32 owner, u8 row, u8 col, i32 max HP, u16 max SP, i32 HP, u16 SP, u8 level, u8 element, u8 reborn, u8 job.
    /// Kinds: 2 a player, 3 a map NPC (the NPC's look), 4 a pet, 7 a monster, 6 / 9 / 11..16 other fighters of Npc.dat.
    /// State: for the own player 1 fighting, 4 watching, 5 waiting for the next bout (100 the same).
    struct FighterRecord {
        int state = 0;
        int kind = 0;
        std::int32_t id = 0;
        int mapNpc = 0;
        std::int32_t ownerId = 0;
        int row = 0;
        int col = 0;
        std::int32_t maxHp = 0;
        int maxSp = 0;
        std::int32_t hp = 0;
        int sp = 0;
        int level = 0;
        int element = 0;
        int reborn = 0;
        int job = 0;
    };

}

#endif
