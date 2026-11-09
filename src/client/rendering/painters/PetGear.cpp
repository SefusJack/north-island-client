#include "client/rendering/painters/PetGear.h"

#include "client/rendering/painters/ItemCells.h"

namespace client {

    namespace {

        constexpr int kGearSlots = 6;

    }

    void showPetGear(GameServices& services, ui::IconGrid& five, ui::IconGrid& sixth, const std::vector<int>& equipment) {
        five.clearCells();
        sixth.clearCells();
        for (const int itemId : equipment) {
            const auto record = services.items.find(itemId);
            if (!record || record->equipSlot < 1 || record->equipSlot > kGearSlots) {
                continue;
            }
            protocol::ItemInstance item;
            item.itemId = itemId;
            item.count = 1;
            (record->equipSlot == kGearSlots ? sixth : five).setCell(record->equipSlot, itemIconCell(services, item, false));
        }
    }

}
