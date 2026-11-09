#include "client/rules/PetWeapon.h"

namespace client {

    namespace {

        constexpr std::size_t kWeaponSlot = 3;
        constexpr int kColorPart = 4;

    }

    PetWeapon petWeapon(const Pet& pet, const formats::ItemTable& items) {
        const protocol::ItemInstance& worn = pet.worn[kWeaponSlot];
        if (worn.empty()) {
            return {};
        }
        return weaponOfItem(worn.lookItemId != 0 ? worn.lookItemId : worn.itemId, pet.info.bodyType, items);
    }

    PetWeapon weaponOfItem(int itemId, int bodyType, const formats::ItemTable& items) {
        if (bodyType < 1 || bodyType > 4) { // pet body types 1..4 are the human ones
            return {};
        }
        const auto record = items.find(itemId);
        if (!record) {
            return {};
        }
        const std::size_t body = static_cast<std::size_t>(bodyType - 1);
        return {record->lookIds[body], record->colorPatterns[kColorPart - 1][body]};
    }

}
