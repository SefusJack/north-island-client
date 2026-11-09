#include "client/rules/Equipping.h"

namespace client {

    namespace {

        constexpr int kFirstEquipmentSlot = 1;
        constexpr int kLastEquipmentSlot = 6;

        /// @brief Tells who may wear an item by its FUsableBy value.
        /// @param usableBy The item's FUsableBy.
        /// @param bodyType The player's body type.
        /// @param element The player's element.
        /// @return True when the body type and element fit.
        bool usableBy(int usableBy, int bodyType, int element) {
            switch (usableBy) {
                case 1: // usable by body types 1 and 3
                    return bodyType == 1 || bodyType == 3;
                case 2: // usable by body types 2 and 4
                    return bodyType == 2 || bodyType == 4;
                case 4: // usable by body type 1
                case 5: // usable by body type 2
                case 6: // usable by body type 3
                case 7: // usable by body type 4
                    return bodyType == usableBy - 3;
                case 12: // usable by element 4 (Wind)
                    return element == 4;
                case 8: // pets only
                case 9: // pets only
                case 13: // pets only
                    return false;
                default: // anyone
                    return true;
            }
        }

    }

    bool isEquipment(const formats::ItemRecord& item) { return item.equipSlot >= kFirstEquipmentSlot && item.equipSlot <= kLastEquipmentSlot; }

    bool canWear(const formats::ItemRecord& item, int bodyType, int element) { return usableBy(item.usableBy, bodyType, element) && item.petNpcId == 0; }

    std::optional<std::string> equipRefusal(const formats::ItemRecord& item, int level, int bodyType, int element) {
        if (item.requiredLevel > level) {
            return "Unable to equip unless level requirements met";
        }
        if (!canWear(item, bodyType, element)) {
            return "Unable to equip this gear";
        }
        return std::nullopt;
    }

}
