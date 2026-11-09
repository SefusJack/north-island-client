#include "client/models/world/NpcFacings.h"

namespace client {

    namespace {

        /// @brief Finds the first place holding an NPC index.
        /// @details Original: FindSavedNpcFacing (index 0 finds a free place).
        /// @param slots The saved facings.
        /// @param npcIndex NPC index to look for.
        /// @return The place number, -1 for none.
        int findSlot(const std::array<NpcFacings::Entry, NpcFacings::kSlots>& slots, int npcIndex) {
            for (int slot = 0; slot < NpcFacings::kSlots; ++slot) {
                if (slots[static_cast<std::size_t>(slot)].npcIndex == npcIndex) {
                    return slot;
                }
            }
            return -1;
        }

    }

    bool NpcFacings::save(int npcIndex, int direction) {
        if (npcIndex == 0 || findSlot(m_slots, npcIndex) >= 0) {
            return false;
        }
        const int free = findSlot(m_slots, 0);
        if (free < 0) {
            return false;
        }
        m_slots[static_cast<std::size_t>(free)] = {npcIndex, direction};
        return true;
    }

    bool NpcFacings::restore(int npcIndex) {
        const int slot = npcIndex == 0 ? -1 : findSlot(m_slots, npcIndex);
        if (slot < 0) {
            return false;
        }
        m_slots[static_cast<std::size_t>(slot)] = {};
        return true;
    }

    std::vector<NpcFacings::Entry> NpcFacings::restoreAll() {
        std::vector<Entry> turned;
        for (const Entry& entry : m_slots) {
            if (entry.npcIndex != 0) {
                turned.push_back(entry);
            }
        }
        if (m_slots[0].npcIndex != 0) {
            m_slots.fill({});
        }
        return turned;
    }

}
