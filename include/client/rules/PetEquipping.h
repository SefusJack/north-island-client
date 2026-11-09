#ifndef NORTH_ISLAND_CLIENT_RULES_PET_EQUIPPING_H
#define NORTH_ISLAND_CLIENT_RULES_PET_EQUIPPING_H

#include "client/models/pets/Pet.h"
#include "formats/ItemRecord.h"
#include "formats/NpcTable.h"

#include <optional>
#include <string>

namespace client {

    /// @brief The pet a career pet became from.
    /// @details Original: PetCareerBaseIds (the old pet kind, when it can grow).
    /// @param npcs The NPC table.
    /// @param npcId The career pet's NPC id.
    /// @return The old pet's NPC id, 0 when the kind has none.
    int petCareerBaseId(const formats::NpcTable& npcs, int npcId);

    /// @brief Why a bag item may not be put on a pet.
    /// @details Original: TEquipManage.EquipBagItem for the pets 1..4. A soldier's gear, a pet in a stall and pets that cannot be dressed (a quest NPC) are refused.
    /// @param item The bag item.
    /// @param pet The pet.
    /// @param npcs The NPC table.
    /// @param inStall Whether the pet is in a stall.
    /// @return The message (empty where the game refuses without a word), or nothing when allowed.
    std::optional<std::string> petEquipRefusal(const formats::ItemRecord& item, const Pet& pet, const formats::NpcTable& npcs, bool inStall);

    /// @brief Why the pet's gear may not be taken off or destroyed (a pet in a stall, a soldier).
    /// @details Original: RequestUnequip for the pets 1..4.
    /// @param pet The pet.
    /// @param inStall Whether the pet is in a stall.
    /// @return The message, or nothing when allowed.
    std::optional<std::string> petUnequipRefusal(const Pet& pet, bool inStall);

}

#endif
