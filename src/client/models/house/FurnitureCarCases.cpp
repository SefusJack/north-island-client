#include "client/models/house/FurnitureCarCases.h"

namespace client {

    void FurnitureCarCases::clear() { m_kept = {}; }

    void FurnitureCarCases::put(const protocol::FurnitureCupboardItem& stored) {
        const int slot = m_utensils.slotOfPiece(stored.furnIndex);
        if (slot != 0) {
            const int itemId = m_utensils.utensil(slot).itemId;
            if (itemId != kGarageItem && itemId != kShowPlatformItem) {
                return;
            }
        }
        if (stored.place < 0 || stored.place > kCars || stored.id < 0 || stored.id > kCars) {
            return;
        }
        m_kept[static_cast<std::size_t>(stored.id)] = stored.item;
        if (slot != 0) {
            m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(stored.place)] = stored.id;
        }
    }

    bool FurnitureCarCases::remove(int furnIndex, int cell, int id) {
        const int slot = m_utensils.slotOfPiece(furnIndex);
        if (slot == 0 || m_utensils.utensil(slot).itemId != kGarageItem) {
            return false;
        }
        if (cell < 0 || cell > kCars || id < 0 || id > kCars) {
            return false;
        }
        m_kept[static_cast<std::size_t>(id)] = protocol::ItemInstance{};
        m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(cell)] = 0;
        return true;
    }

    const protocol::ItemInstance& FurnitureCarCases::keptUnder(int id) const {
        static const protocol::ItemInstance none;
        return id >= 1 && id <= kCars ? m_kept[static_cast<std::size_t>(id)] : none;
    }

    int FurnitureCarCases::emptyNumber() const {
        for (int id = 1; id <= kCars; ++id) {
            if (m_kept[static_cast<std::size_t>(id)].empty()) {
                return id;
            }
        }
        return 0;
    }

}
