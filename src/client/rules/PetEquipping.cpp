#include "client/rules/PetEquipping.h"

#include <array>

namespace client {

    namespace {

        constexpr int kFirstEquipmentSlot = 1; // equip slots 1..6 are gear
        constexpr int kLastEquipmentSlot = 6; // last gear slot
        constexpr int kCareerItemType = 0x95; // item sub type 0x95 (149): pet career items
        constexpr int kNotForPets = 64; // item flag bit 64: cannot be equipped by pets
        constexpr int kElinPet = 14230; // NPC 14230: Elin
        constexpr int kRebornElinPet = 31029; // NPC 31029: Elin (reborn)
        constexpr int kElinWeaponSlot = 3; // equip slot 3: weapon slot used for Elin's plasma weapons
        // saddle item ids: Swift, Comfortable, Bash, Magic, Defence, Spirit, Decelerate, Tenacity
        constexpr std::array<int, 8> kSaddles = {25208, 25214, 25535, 25536, 25539, 25625, 25540, 25641};
        // pet NPC ids per row: {base pet (growKey table), career pet}
        constexpr std::array<std::array<int, 2>, 13> kCareers = {
            {{14156, 31027}, // Xaolan -> Xaolan
             {14081, 31026}, // Niss -> Niss
             {14162, 31025}, // Roca -> Roca
             {14175, 31028}, // Clive -> Clive
             {14615, 31035}, // Qlaya -> Qlaya
             {12068, 31037}, // Magellan -> Magellan
             {14609, 31034}, // Shasha -> Shasha
             {14230, 31029}, // Elin -> Elin
             {15689, 31040}, // Hayate -> Hayate
             {14548, 31033}, // Kanako -> Kanako
             {12095, 31038}, // Frederico -> Frederico
             {15657, 31039}, // Victoria -> Victoria
             {14274, 31032}} // Sam -> Sam
        };

        /// @brief Whether an item is a saddle.
        /// @details Original: IsGetSaddleKind.
        /// @param itemId The item id.
        /// @return True for a saddle.
        bool isSaddle(int itemId) {
            for (const int saddle : kSaddles) {
                if (saddle == itemId) {
                    return true;
                }
            }
            return false;
        }

    }

    int petCareerBaseId(const formats::NpcTable& npcs, int npcId) {
        for (const auto& career : kCareers) {
            if (career[1] != npcId) {
                continue;
            }
            const auto old = npcs.find(career[0]);
            return old && old->growKey != 0 ? career[0] : 0;
        }
        return 0;
    }

    std::optional<std::string> petEquipRefusal(const formats::ItemRecord& item, const Pet& pet, const formats::NpcTable& npcs, bool inStall) {
        constexpr const char* kCannotEquip = "Unable to equip this gear";
        if (item.petNpcId == 0 && item.requiredLevel > pet.level) {
            return "Unable to equip unless level requirements met";
        }
        bool fail = false;
        if (item.petNpcId == 0) {
            switch (item.usableBy) {
                case 0: // usableBy 0: any pet
                case 8: // usableBy 8: any pet
                    break;
                case 9: // usableBy 9: pets with direction kind 0
                case 10: // usableBy 10: pets with direction kind 0
                    fail = pet.info.directionKind != 0; // direction kind 0
                    break;
                case 11: // usableBy 11: pets with direction kind 1
                case 13: // usableBy 13: pets with direction kind 1
                    fail = pet.info.directionKind != 1;
                    break;
                case 12: // usableBy 12: pets of element 4 (Wind)
                    fail = pet.info.element != 4; // element 4: Wind
                    break;
                default: // any other usableBy: not for pets
                    fail = true;
                    break;
            }
            if (isSaddle(item.id) && pet.info.directionKind != 0) {
                fail = false;
            }
            if (pet.info.mercenary != 0) {
                return "Soldiers equipment can't be changed";
            }
        } else if (pet.info.npcId != item.petNpcId) {
            fail = !(item.subType == kCareerItemType && petCareerBaseId(npcs, pet.info.npcId) == item.petNpcId);
        }
        if (fail) {
            return kCannotEquip;
        }
        if (pet.info.npcId == kElinPet && item.equipSlot == kElinWeaponSlot && item.id != 20044 &&
            item.id != 20045) { // item ids 20044 (Plasma Gun), 20045 (Plasma Wafer): the plasma weapons of the Elin pet
            return kCannotEquip;
        }
        if (pet.info.npcId == kRebornElinPet && item.equipSlot == kElinWeaponSlot &&
            item.id != 20048) { // item id 20048 (Plasma Wafer): the plasma weapon of the reborn Elin pet
            return kCannotEquip;
        }
        if (inStall) {
            return "Unable to change equipment in selling";
        }
        if (item.equipSlot < kFirstEquipmentSlot || item.equipSlot > kLastEquipmentSlot || pet.tradable != 1) {
            return std::string();
        }
        if ((item.flags & kNotForPets) != 0) {
            return "Unable to equip the pet";
        }
        return std::nullopt;
    }

    std::optional<std::string> petUnequipRefusal(const Pet& pet, bool inStall) {
        if (inStall) {
            return "Unable to change equipment in selling";
        }
        if (pet.info.mercenary != 0) {
            return "Soldiers equipment can't be changed";
        }
        return std::nullopt;
    }

}
