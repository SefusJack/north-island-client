#include "client/models/house/FurnitureCupboards.h"

#include "client/rules/FurnitureKinds.h"

namespace client {

    void FurnitureCupboards::clear() { m_kept = {}; }

    void FurnitureCupboards::put(const protocol::FurnitureCupboardItem& stored) {
        const int slot = m_utensils.slotOfPiece(stored.furnIndex);
        if (slot != 0 && FurnitureKinds::cupboardKind(m_items, m_utensils.utensil(slot).itemId) == 0) {
            return;
        }
        if (stored.id < 1 || stored.id > kItems || stored.place < 1 || stored.place > kPlaces) {
            return;
        }
        if (slot != 0 && m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(stored.place)] != 0) {
            return;
        }
        m_kept[static_cast<std::size_t>(stored.id)] = {stored.furnIndex, stored.place, stored.item};
        if (slot != 0) {
            m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(stored.place)] = stored.id;
        }
    }

    void FurnitureCupboards::remove(int furnIndex, int place, int id) {
        const int slot = m_utensils.slotOfPiece(furnIndex);
        if (slot == 0 || FurnitureKinds::cupboardKind(m_items, m_utensils.utensil(slot).itemId) == 0) {
            return;
        }
        if (id < 1 || id > kItems || place < 1 || place > kPlaces) {
            return;
        }
        m_kept[static_cast<std::size_t>(id)] = {};
        m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(place)] = 0;
    }

    void FurnitureCupboards::relink(int furnIndex) {
        const int slot = m_utensils.slotOfPiece(furnIndex);
        if (slot == 0 || FurnitureKinds::cupboardKind(m_items, m_utensils.utensil(slot).itemId) == 0) {
            return;
        }
        for (int id = 1; id <= kItems; ++id) {
            const Kept& kept = m_kept[static_cast<std::size_t>(id)];
            if (kept.furnIndex != 0 && kept.furnIndex == furnIndex && kept.place >= 1 && kept.place <= kPlaces) {
                m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(kept.place)] = id;
            }
        }
    }

    const protocol::ItemInstance& FurnitureCupboards::itemAt(int furnIndex, int place) const {
        static const protocol::ItemInstance none;
        const int slot = m_utensils.slotOfPiece(furnIndex);
        if (slot == 0 || place < 1 || place > kPlaces) {
            return none;
        }
        const int id = m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(place)];
        return id >= 1 && id <= kItems ? m_kept[static_cast<std::size_t>(id)].item : none;
    }

    int FurnitureCupboards::emptyPlace(int furnIndex) const {
        const int slot = m_utensils.slotOfPiece(furnIndex);
        if (slot == 0) {
            return 0;
        }
        for (int place = 1; place <= kPlaces; ++place) {
            if (m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(place)] == 0) {
                return place;
            }
        }
        return 0;
    }

}
