#include "client/data/CareerLooks.h"

#include "client/data/CareerTexts.h"

#include <array>

namespace client {

    namespace {

        constexpr int kNoLook = 1000; // look id 1000: the item has no sprite for this body type

        // item ids per class (index 0 unused); per row: headgear, clothes, slot 3, shoes, weapon
        const std::array<std::array<int, 5>, 7> kClassItems = {{
            {},
            {22061, 21075, 23081, 24076, 10052}, // Killer: Headband, Masked Suit, Iga Gloves, Sung Boots, Katana
            {22102, 21129, 23107, 24071, 11055}, // Warrior: Fighting Helmet, Copper Armor, Gold Bracers, Beast Boots, Skeleton Steel Sword
            {22081, 21098, 23089, 24082, 12044}, // Knight: Footman's Helmet, Steel Armor, Steel Cuffs, Copper Winged Boots, Long Spear
            {22088, 21112, 23085, 24076, 14052}, // Wit: Leather Hat, Demon Gown, Magic Gloves, Sung Boots, Whirlpool Wand
            {22202, 21194, 23148, 24156, 14062}, // Priest: Priest Cap, Priest's Clothes, Priest's Gloves, Priest Boots, Defence Wand
            {22203, 21195, 23149, 24157, 14105}, // Seer: Seer Cap, Seer's Clothes, Seer's Gloves, Seer Boots, Seer Wand
        }};

    }

    CareerModel makeCareerModel(
        const CharacterLook& player,
        const formats::ItemTable& items,
        const std::string& name,
        int career,
        const std::vector<int>& itemIds
    ) {
        CareerModel model;
        model.name = name;
        model.career = career;
        model.pose = kCareerRestPose;
        CharacterLook& look = model.look;
        look.bodyType = player.bodyType;
        look.head = player.head;
        look.bodySprite = player.bodySprite;
        look.equipmentLooks = player.equipmentLooks;
        look.equipmentLooks[6] = 0; // slot 6 is the special slot: nothing in it
        const std::size_t bodyIndex = static_cast<std::size_t>(look.bodyType - 1);
        for (const int itemId : itemIds) {
            const auto item = items.find(itemId);
            if (item && item->equipSlot >= 1 && item->equipSlot <= 6) { // equipment slots 1..6
                const int lookId = item->lookIds[bodyIndex];
                look.equipmentLooks[static_cast<std::size_t>(item->equipSlot)] = lookId == kNoLook ? 0 : lookId;
            }
        }
        return model;
    }

    CareerModel makeCareerClassModel(const CharacterLook& player, const formats::ItemTable& items, int career) {
        const auto& clothes = kClassItems.at(static_cast<std::size_t>(career));
        CareerModel model = makeCareerModel(player, items, careerName(career), career, {clothes.begin(), clothes.end()});
        if (career == 5) { // career 5: the priest
            const bool man = player.bodyType == 1 || player.bodyType == 3; // body types 1 and 3 are male
            // nine-digit colour pattern codes (one digit per colour block, 4 = unchanged); packed colour patterns for part 2 (man / woman), then part 3 (man / woman)
            model.look.pattern.apply(man ? 444975200 : 444965424, 2);
            model.look.pattern.apply(man ? 655444777 : 444444589, 3);
        }
        return model;
    }

    CareerModel makeBeforeCareerModel(const CharacterLook& player, const formats::ItemTable& items, int slot) {
        const int career = slot == 1 ? 1 : 4; // slot 1: soldier uses the killer's clothes (1), the mage the wit's (4)
        const auto& clothes = kClassItems.at(static_cast<std::size_t>(career));
        return makeCareerModel(player, items, slot == 1 ? "Soldier" : "Mage", career, {clothes.begin(), clothes.end()});
    }

}
