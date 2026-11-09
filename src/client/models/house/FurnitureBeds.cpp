#include "client/models/house/FurnitureBeds.h"

#include "client/rules/FurnitureKinds.h"

namespace client {

    void FurnitureBeds::clear() { m_kept = {}; }

    void FurnitureBeds::put(const protocol::FurnitureBedPet& stored) {
        const int slot = m_utensils.slotOfPiece(stored.furnIndex);
        const int kind = FurnitureKinds::bedKind(m_items, m_utensils.utensil(slot).itemId);
        if (kind == 0 || stored.bed < 1 || stored.bed > kind || m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(stored.bed)] != 0) {
            return;
        }
        if (stored.id < 1 || stored.id > kPets) {
            return;
        }
        m_kept[static_cast<std::size_t>(stored.id)] = {stored.furnIndex, stored.bed, stored.pet};
        m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(stored.bed)] = stored.id;
    }

    std::optional<protocol::PetRecord> FurnitureBeds::release(int furnIndex, int bed, int id) {
        const int slot = m_utensils.slotOfPiece(furnIndex);
        const int kind = FurnitureKinds::bedKind(m_items, m_utensils.utensil(slot).itemId);
        if (kind == 0 || bed < 1 || bed > kind || m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(bed)] == 0) {
            return std::nullopt;
        }
        if (id < 1 || id > kPets) {
            return std::nullopt;
        }
        const protocol::PetRecord pet = m_kept[static_cast<std::size_t>(id)].pet;
        m_kept[static_cast<std::size_t>(id)] = {};
        m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(bed)] = 0;
        return pet;
    }

    void FurnitureBeds::relink(int furnIndex) {
        const int slot = m_utensils.slotOfPiece(furnIndex);
        if (slot == 0 || FurnitureKinds::bedKind(m_items, m_utensils.utensil(slot).itemId) == 0) {
            return;
        }
        for (int id = 1; id <= kPets; ++id) {
            const Kept& kept = m_kept[static_cast<std::size_t>(id)];
            if (kept.pet.npcId != 0 && kept.furnIndex == furnIndex && kept.bed >= 1 && kept.bed <= 10) { // highest content slot number of a piece
                m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(kept.bed)] = id;
            }
        }
    }

    std::optional<std::reference_wrapper<const protocol::PetRecord>> FurnitureBeds::petIn(int furnIndex, int bed) const {
        const int slot = m_utensils.slotOfPiece(furnIndex);
        if (slot == 0 || bed < 1 || bed > 10) { // highest content slot number of a piece
            return std::nullopt;
        }
        const int id = m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(bed)];
        if (id < 1 || id > kPets) {
            return std::nullopt;
        }
        const Kept& kept = m_kept[static_cast<std::size_t>(id)];
        if (kept.pet.npcId == 0 || kept.furnIndex != furnIndex) {
            return std::nullopt;
        }
        return std::cref(kept.pet);
    }

    int FurnitureBeds::bedOfSleeper(int furnIndex, int order) const {
        const int slot = m_utensils.slotOfPiece(furnIndex);
        const int kind = FurnitureKinds::bedKind(m_items, m_utensils.utensil(slot).itemId);
        int seen = 0;
        for (int bed = 1; bed <= kind; ++bed) {
            if (m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(bed)] != 0) {
                ++seen;
            }
            if (seen == order) {
                return petIn(furnIndex, bed) ? bed : 0;
            }
        }
        return 0;
    }

}
