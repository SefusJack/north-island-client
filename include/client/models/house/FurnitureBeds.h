#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_FURNITURE_BEDS_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_FURNITURE_BEDS_H

#include "client/models/house/HouseUtensils.h"
#include "formats/ItemTable.h"
#include "protocol/FurnitureBedPet.h"
#include "protocol/PetRecord.h"

#include <array>
#include <functional>
#include <optional>

namespace client {

    /// @brief The pets asleep in the beds of a house, up to six, each in a bed (1..4) of a piece that the piece's content slots point to.
    /// @details Original: DZ_Thing.pas TUtensilManage.FBedPets with what HandleUtensilPacket's 62/43 and 62/44 do to them.
    class FurnitureBeds {
        public:
            static constexpr int kPets = 6;

            /// @brief Creates the bed list over the item table and the house utensils.
            /// @param items Item table (to tell beds).
            /// @param utensils The house's furniture pieces.
            FurnitureBeds(
                const formats::ItemTable& items,
                HouseUtensils& utensils
            )
                : m_items(items),
                  m_utensils(utensils) {}

            /// @brief Forgets all sleeping pets.
            void clear();

            /// @brief A pet put to sleep (a bed that is taken, or one the piece does not have, refuses it).
            /// @details Original: server packet 62/43.
            /// @param stored The pet, its piece and bed.
            void put(const protocol::FurnitureBedPet& stored);

            /// @brief The pet in that bed (kept under that number) leaves it.
            /// @details Original: server packet 62/44.
            /// @param furnIndex Furniture piece index.
            /// @param bed Bed number.
            /// @param id Number the pet is kept under.
            /// @return The pet, or nothing when there is none.
            std::optional<protocol::PetRecord> release(int furnIndex, int bed, int id);

            /// @brief Points the beds of the piece to the pets kept for it again.
            /// @details Original: the tail of AddHomeUtensil in the list handler.
            /// @param furnIndex Furniture piece index.
            void relink(int furnIndex);

            /// @brief The pet in a bed of a piece.
            /// @param furnIndex Furniture piece index.
            /// @param bed Bed number.
            /// @return The pet, or nothing for an empty bed.
            std::optional<std::reference_wrapper<const protocol::PetRecord>> petIn(int furnIndex, int bed) const;

            /// @brief The bed holding the n-th sleeper of the piece.
            /// @details Original: GetBedPetSlot.
            /// @param furnIndex Furniture piece index.
            /// @param order Which sleeper, from 1.
            /// @return Bed number, 0 when there are not that many.
            int bedOfSleeper(int furnIndex, int order) const;

        private:
            /// @brief A pet kept for a bed: the piece, the bed and the pet (an npc id of 0 means none).
            struct Kept {
                int furnIndex = 0;
                int bed = 0;
                protocol::PetRecord pet;
            };

            const formats::ItemTable& m_items;
            HouseUtensils& m_utensils;
            std::array<Kept, kPets + 1> m_kept{};
    };

}

#endif
