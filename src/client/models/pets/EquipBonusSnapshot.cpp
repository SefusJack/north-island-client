#include "client/models/pets/EquipBonusSnapshot.h"

#include <cstdlib>

namespace client {

    namespace {

        /// @brief One bonus line: which snapshot value and its display name.
        struct Line {
            std::size_t index;
            const char* name;
        };

        constexpr std::array<Line, 7> kLines = {
            {{2, "Attack"}, {3, "Defense"}, {4, "Swift"}, {0, "Mana Attack"}, {1, "Mana Defense"}, {5, "Maximum HP"}, {6, "Maximum SP"}}
        };

    }

    std::array<int, 7> EquipBonusSnapshot::valuesOf(const Pet& pet) {
        // combatBonus indices 2,3,0,1,4 reorder ATK,DEF,MATK,MDEF,SPD into MATK,MDEF,ATK,DEF,SPD order
        return {pet.combatBonus[2], pet.combatBonus[3], pet.combatBonus[0], pet.combatBonus[1], pet.combatBonus[4], pet.maxHpBonus, pet.maxSpBonus};
    }

    EquipBonusSnapshot EquipBonusSnapshot::of(const Pet& pet) {
        EquipBonusSnapshot snapshot;
        snapshot.m_values = valuesOf(pet);
        return snapshot;
    }

    std::string EquipBonusSnapshot::changeLines(const Pet& now) const {
        const std::array<int, 7> current = valuesOf(now);
        std::string lines;
        for (const Line& line : kLines) {
            const int diff = current[line.index] - m_values[line.index];
            if (diff != 0) {
                lines += std::string("\r") + line.name + (diff > 0 ? " increases " : " decreases ") + std::to_string(std::abs(diff));
            }
        }
        return lines;
    }

}
