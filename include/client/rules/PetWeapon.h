#ifndef NORTH_ISLAND_CLIENT_RULES_PET_WEAPON_H
#define NORTH_ISLAND_CLIENT_RULES_PET_WEAPON_H

#include "client/models/pets/Pet.h"
#include "formats/ItemTable.h"

#include <cstdint>

namespace client {

    /// @brief What a pet of a human body type holds in its hand.
    /// @details Original: DrawHumanLayers, FHumKind 4 (part 4 colour code).
    struct PetWeapon {
        int lookId = 0;
        std::int32_t colorCode = 0;
    };

    /// @brief The weapon a pet holds.
    /// @param pet The pet.
    /// @param items The item table.
    /// @return The weapon's look id and colour code; empty for nothing to draw.
    PetWeapon petWeapon(const Pet& pet, const formats::ItemTable& items);

    /// @brief The weapon of an item for a pet body type.
    /// @param itemId The item id.
    /// @param bodyType The pet body type (1..4).
    /// @param items The item table.
    /// @return The look id and part 4 colour code; empty for none.
    PetWeapon weaponOfItem(int itemId, int bodyType, const formats::ItemTable& items);

}

#endif
