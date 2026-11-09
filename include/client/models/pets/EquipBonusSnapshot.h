#ifndef NORTH_ISLAND_CLIENT_MODELS_PETS_EQUIP_BONUS_SNAPSHOT_H
#define NORTH_ISLAND_CLIENT_MODELS_PETS_EQUIP_BONUS_SNAPSHOT_H

#include "client/models/pets/Pet.h"

#include <array>
#include <string>

namespace client {

    /// @brief The bonuses of a pet when an equip or unequip request goes out, to name what the item added or took away.
    /// @details Original: EquipBonusSnapshot; ShowEquipChangeMsg names the change ("Attack increases 2") once the 8/2 attributes of the reply have arrived.
    class EquipBonusSnapshot {
        public:
            /// @brief Takes the snapshot of a pet.
            /// @param pet The pet.
            /// @return The snapshot.
            static EquipBonusSnapshot of(const Pet& pet);

            /// @brief One line (led by a carriage return) for each bonus that differs, in the order Attack, Defense, Swift, Mana Attack, Mana Defense, Maximum HP, Maximum SP.
            /// @details Original: ShowEquipChangeMsg.
            /// @param now The pet after the change.
            /// @return The lines, empty when nothing differs.
            std::string changeLines(const Pet& now) const;

        private:
            std::array<int, 7> m_values{};

            /// @brief The bonuses of a pet in the order MATK, MDEF, ATK, DEF, SPD, max HP, max SP.
            /// @param pet The pet.
            /// @return The seven values.
            static std::array<int, 7> valuesOf(const Pet& pet);
    };

}

#endif
