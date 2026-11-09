#include "client/rules/Compounding.h"

namespace client {

    namespace {

        constexpr int kNoCompoundFlag = 4; // item flag bit 0x04: the item may not be compounded
        constexpr int kVehicleType = 39; // item type 39: vehicles and their capsules (Airship, Boat, Cabriolet, ...)
        constexpr int kJobManteauSubType = 142; // item sub type 142: job manteau (allowed without an alchemy class)
        constexpr int kRefusedItem = 61062; // item 61062: Chaos Crystal (refused as an ingredient)

    }

    int compoundRefusal(const formats::ItemTable& items, int itemId) {
        const auto item = items.find(itemId);
        if (!item || item->unlisted != 0) {
            return 255; // refusal 255: no such item
        }
        if ((item->flags & kNoCompoundFlag) != 0) {
            return 1; // refusal 1: marked so
        }
        if (item->cellWidth != 1) {
            return 2; // refusal 2: larger than a cell
        }
        if (item->type == kVehicleType) {
            return 3; // refusal 3: a vehicle
        }
        if (item->combineKind == 0 && item->subType != kJobManteauSubType) {
            return 4; // refusal 4: no alchemy class
        }
        if (item->id == kRefusedItem) {
            return 6; // refusal 6: item 61062
        }
        return 0;
    }

}
