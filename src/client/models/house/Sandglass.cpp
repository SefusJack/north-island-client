#include "client/models/house/Sandglass.h"

namespace client {

    namespace {

        constexpr int kWholeAmount = 250; // used-amount value of a fully used Magic Sandglass

    }

    bool Sandglass::isSandglassItem(const formats::ItemTable& items, int itemId) {
        return (itemId == 39074 || itemId == 39097) && items.find(itemId).has_value(); // item ids of the two Magic Sandglass pieces
    }

    std::string Sandglass::remainingLabel(int maxUses, int usedAmount) {
        if (maxUses == 0) {
            return {};
        }
        const int step = kWholeAmount / maxUses;
        return step == 0 ? std::string() : "(Remaining times:" + std::to_string((kWholeAmount - usedAmount) / step) + "Times)";
    }

    bool Sandglass::slotActive(int slot) const {
        return slot >= 1 && slot <= 8 && (m_flags & (1 << (slot - 1))) != 0; // job slots 1..8, one flag bit each
    }

}
