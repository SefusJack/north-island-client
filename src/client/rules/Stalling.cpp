#include "client/rules/Stalling.h"

#include "client/rules/Equipping.h"

namespace client {

    namespace {

        constexpr int kNoTradeFlag = 2; // item record flag bit value 2: the item cannot be traded
        constexpr int kNoNpcSaleFlag = 16; // item record flag bit value 16: the item cannot be sold to an NPC shop
        constexpr int kAccessoryType = 28; // item type 28: gems, tools and similar (e.g. Immaculate Ruby, AC Generator)
        constexpr int kInstrumentType = 29; // item type 29: workshop machines (e.g. Auto Loom, Bench Saw)
        constexpr int kFurnitureType = 30; // item type 30: house furniture (e.g. ATM, Air Condition)
        constexpr int kVehicleType = 39; // item type 39: vehicles and vehicle capsules (e.g. Airship, Boat)
        constexpr int kCastleDefenceType = 53; // item type 53: castle defence items (e.g. Gold Eagle Totem, Torch)
        constexpr int kFirstNonTransferable = 61063; // item 61063: Morning Star Crystal, first of the untransferable crystals
        constexpr int kLastNonTransferable = 61078; // item 61078: Charm Crystal, last of the untransferable crystals
        constexpr int kRefused = 255; // result code 255: not handled here at all (1 = may not change hands, 0 = allowed)

    }

    int itemStallRefusal(const formats::ItemTable& items, int itemId) {
        const auto item = items.find(itemId);
        if (!item) {
            return kRefused;
        }
        if ((item->flags & kNoTradeFlag) != 0) {
            return 1;
        }
        const bool large = item->cellWidth != 1 || item->cellHeight != 1;
        const bool largeKind = item->type == kInstrumentType || item->type == kFurnitureType || item->type == kAccessoryType;
        if ((largeKind && large) || item->type == kVehicleType || item->type == kCastleDefenceType) {
            return kRefused;
        }
        return 0;
    }

    int npcSaleRefusal(const formats::ItemTable& items, int itemId, bool equipmentShop) {
        const auto item = items.find(itemId);
        if (!item) {
            return kRefused;
        }
        if ((item->flags & kNoNpcSaleFlag) != 0) {
            return 1;
        }
        if (equipmentShop || isEquipment(*item)) {
            return equipmentShop && isEquipment(*item) ? 0 : kRefused;
        }
        const bool large = item->cellWidth != 1 || item->cellHeight != 1;
        const bool largeKind = item->type == kInstrumentType || item->type == kFurnitureType || item->type == kAccessoryType;
        if ((largeKind && large) || item->type == kVehicleType) {
            return kRefused;
        }
        return 0;
    }

    bool isNonTransferable(int itemId) { return itemId >= kFirstNonTransferable && itemId <= kLastNonTransferable; }

}
