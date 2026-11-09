#include "client/models/character/CharacterLook.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kNoLook = 1000; // look id 1000 in the item table means "no sprite" (stored as 0)

        /// @brief Whether a look is one of the special-slot looks that change how a figure is drawn.
        /// @details Original: GetItemLookSlot; the 2331 / 2232 / 2332..2335 / 2347 family of each body type.
        /// @param look Look (sprite) id.
        /// @return True for a special-slot look.
        bool isSpecialSlotLook(int look) {
            const int tail = look % 1000; // the per-body-type family is the last three digits
            // look ids 2000..5999 are the four body types; tails 331, 232, 332..335, 347 are the special-slot looks
            return look >= 2000 && look < 6000 && (tail == 331 || tail == 232 || (tail >= 332 && tail <= 335) || tail == 347);
        }

    }

    CharacterLook CharacterLook::fromSummary(const protocol::CharacterSummary& character, const formats::ItemTable& items) {
        formats::ColorPattern colors;
        colors.apply(character.colorPattern1, 1); // colour part 1
        colors.apply(character.colorPattern2, 5); // colour part 5
        CharacterLook look = dressed(character.bodyType, character.head, colors, character.equipment, items);
        look.job = character.job;
        return look;
    }

    CharacterLook CharacterLook::fromAppearance(const protocol::PlayerAppearance& appearance, const formats::ItemTable& items, int job) {
        formats::ColorPattern colors;
        colors.apply(appearance.colorCode1, 1); // colour part 1
        colors.apply(appearance.colorCode2, 5); // colour part 5
        std::array<std::uint16_t, 6> equipment{}; // 6 equipment slots
        std::copy_n(appearance.equipment.begin(), std::min(equipment.size(), appearance.equipment.size()), equipment.begin());
        CharacterLook look = dressed(appearance.bodyType, appearance.head, colors, equipment, items);
        look.bodySprite = appearance.bodySprite;
        look.job = job;
        return look;
    }

    CharacterLook CharacterLook::fromColorCodes(
        int bodyType,
        int head,
        std::int32_t code1,
        std::int32_t code5,
        const formats::ItemTable& items
    ) {
        formats::ColorPattern colors;
        colors.apply(code1, 1); // colour part 1
        colors.apply(code5, 5); // colour part 5
        return dressed(bodyType, head, colors, {}, items);
    }

    CharacterLook CharacterLook::dressed(
        int bodyType,
        int head,
        const formats::ColorPattern& colors,
        const std::array<std::uint16_t, 6>& equipment,
        const formats::ItemTable& items
    ) {
        CharacterLook look;
        look.bodyType = bodyType >= 1 && bodyType <= 4 ? bodyType : 1; // body types 1..4
        look.head = head;
        look.bodySprite = 0;
        look.basePattern = colors;
        look.pattern = colors;

        const std::size_t bodyIndex = static_cast<std::size_t>(look.bodyType - 1);
        for (std::uint16_t itemId : equipment) {
            if (itemId == 0) {
                continue;
            }
            const auto item = items.find(itemId);
            // equipment slots 1..6
            if (!item || item->equipSlot < 1 || item->equipSlot > 6) {
                continue;
            }
            const int lookId = item->lookIds[bodyIndex];
            look.equipmentLooks[static_cast<std::size_t>(item->equipSlot)] = lookId == kNoLook ? 0 : lookId;
            if (item->equipSlot == 2) { // slot 2: clothes
                look.weaponKind = item->weaponKind;
            }
            if (item->equipSlot == 6 && isSpecialSlotLook(lookId)) { // slot 6: the special slot
                look.specialType = item->type;
            }
            for (int part = 1; part <= 4; ++part) { // colour parts 1..4 of an item
                look.pattern.apply(item->colorPatterns[static_cast<std::size_t>(part - 1)][bodyIndex], part, item->equipSlot);
            }
        }
        look.drawLook = look.specialType != 0 ? look.equipmentLooks[6] : look.equipmentLooks[2]; // slot 6 special, else slot 2 clothes
        return look;
    }

}
