#include "client/rules/StallDropRules.h"

#include "client/rules/Stalling.h"

namespace client {

    namespace {

        constexpr int kNoTradeFlag = 2; // item record flag bit value 2: the item cannot be traded
        constexpr int kFirstCapsule = 0xBB93; // item 48019: Air Balloon Capsule, first of the vehicle capsules
        constexpr int kLastCapsule = 0xBBA2; // item 48034: Space Craft Capsule, last of the vehicle capsules
        constexpr int kAllowedMark = 1; // bag use mark 1 (compound window): the one use mark that does not block dropping into the stall

    }

    std::optional<StallDropRules::Refusal> StallDropRules::check(int bagSlot, bool owner, bool full, bool vending) const {
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_items.find(item.itemId);
        if (!record) {
            return Refusal{};
        }
        if ((record->flags & kNoTradeFlag) != 0) {
            return Refusal{"It cannot be sold", 2000}; // message duration in ms
        }
        if (!owner) {
            return Refusal{"You're not the owner, cannot sell item here", 1000}; // message duration in ms
        }
        if (full) {
            return Refusal{"It's full", 1000}; // message duration in ms
        }
        if (record->cellWidth != 1 || record->cellHeight != 1) {
            return Refusal{"The item is too big to sell", 1000}; // message duration in ms
        }
        if (const auto lock = m_marks.lockNotice(bagSlot, kAllowedMark)) {
            return Refusal{lock->text, lock->ms};
        }
        if (!m_marks.isAvailable(bagSlot)) {
            return Refusal{"Unable to operate when the item is locked", 1000}; // message duration in ms
        }
        if (item.lookItemId != 0) {
            return Refusal{"It cannot be sold", 2000}; // message duration in ms
        }
        if (isNonTransferable(item.insertItemId)) {
            // message duration in ms (2000 for both texts)
            return vending ? Refusal{"This item cannot be transferred", 2000} : Refusal{"It cannot be sold", 2000};
        }
        if (item.itemId >= kFirstCapsule && item.itemId <= kLastCapsule) {
            return Refusal{"The item can't be stored", 2000}; // message duration in ms
        }
        return std::nullopt;
    }

}
