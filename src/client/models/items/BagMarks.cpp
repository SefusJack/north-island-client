#include "client/models/items/BagMarks.h"

namespace client {

    void BagMarks::clearUseMarks(int mark) {
        for (int& value : m_marks) {
            if (value == mark) {
                value = 0;
            }
        }
    }

    std::optional<BagMarks::LockNotice> BagMarks::lockNotice(int slot, int allowedMark) const {
        if (slot < 1 || slot > kSlots) {
            return LockNotice{};
        }
        const int mark = useMark(slot);
        if (mark == 0 || mark == allowedMark) {
            return std::nullopt;
        }
        const std::string locked = "Unable to operate when the item is locked";
        // the number in each notice is its display time in ms
        switch (mark) {
            case 1: return LockNotice{"Operation unavailable for items ready for compounding", 2000}; // mark 1: compound window
            case 2: return LockNotice{"Operation unavailable for items ready for trading", 2000}; // mark 2: trade window
            case 3: return LockNotice{locked, 2000}; // mark 3: sale
            case 4: return LockNotice{"Items are on sale, this action cannot performed.", 2000}; // mark 4: own stall
            case 5: return LockNotice{"Operation unavailable while selecting banks", 2000}; // mark 5: bank
            case 6: return LockNotice{"Operation unavailable for items ready for express", 2000}; // mark 6: express (mail)
            case 7: // mark 7: repair
            case 8: return LockNotice{locked, 2000}; // mark 8: forge
            case 9: return LockNotice{"Operation unavailable for fishing rod in use", 2000}; // mark 9: fishing rod in use
            case 10: return LockNotice{"Operation unavailable while auto remote controller still in use", 2000}; // mark 10: remote control in use
            case 11: return LockNotice{"Repairing the item", 1000}; // mark 11: item repair
            case 13: return LockNotice{"Disassembling, cannot do the movement", 1000}; // mark 13: dismantle
            default: return LockNotice{locked, 1000}; // any other mark
        }
    }

}
