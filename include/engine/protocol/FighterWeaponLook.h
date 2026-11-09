#ifndef NORTH_ISLAND_PROTOCOL_FIGHTER_WEAPON_LOOK_H
#define NORTH_ISLAND_PROTOCOL_FIGHTER_WEAPON_LOOK_H

#include <cstdint>

namespace protocol {

    /// @brief One 12-byte record of 11/251: the weapon a pet in a battle holds, which its fighter record cannot carry.
    /// @details Original: RecvFighterEquipLook.
    struct FighterWeaponLook {
        std::int32_t id = 0;
        std::int32_t ownerId = 0;
        int row = 0;
        int col = 0;
        int itemId = 0;
    };

}

#endif
