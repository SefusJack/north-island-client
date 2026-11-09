#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_FIELD_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_FIELD_H

#include "client/subsystems/battle/Fighter.h"
#include "client/subsystems/battle/ScreenEffects.h"
#include "core/Point.h"

#include <array>
#include <functional>
#include <optional>

namespace client {

    /// @brief The fighters of a battle by slot (1..16, the own player always slot 1) on the 4 x 4 grid.
    /// @details Original: TFightField.FFightHums, InitFightGridPos.
    class BattleField {
        public:
            static constexpr int kSlots = 16; // fighter slots 1..16 (4 x 4 grid)

            /// @brief The feet of a fighter on a cell: rows 1 and 2 on the left, rows 3 and 4 on the right, each column further down and towards the middle.
            /// @param row Grid row.
            /// @param col Grid column.
            /// @return Screen position of the feet.
            static core::Point cellPosition(int row, int col);

            /// @brief Removes all fighters.
            void clear();

            /// @brief Allocates a fighter slot: the own player's slot 1, otherwise the first free one.
            /// @details Original: AllocFightHumSlot.
            /// @param ownPlayer Whether the fighter is the own player.
            /// @return Slot, 0 when full.
            int freeSlot(bool ownPlayer) const;

            /// @brief Puts a fighter in a slot.
            /// @param slot Slot number.
            /// @param fighter The fighter.
            void place(int slot, Fighter fighter);

            /// @brief Removes the fighter in a slot.
            /// @param slot Slot number.
            void remove(int slot) { m_fighters.at(static_cast<std::size_t>(slot)).reset(); }

            ScreenEffects effects;

            /// @brief The fighter in a slot.
            /// @param slot Slot number.
            /// @return The fighter, or nothing when the slot is empty.
            std::optional<std::reference_wrapper<Fighter>> fighter(int slot);

            /// @brief The fighter in a slot (const).
            /// @param slot Slot number.
            /// @return The fighter, or nothing when the slot is empty.
            std::optional<std::reference_wrapper<const Fighter>> fighter(int slot) const;

            /// @brief The slot of the fighter at a cell.
            /// @details Original: GetCellFighterSlot.
            /// @param row Grid row.
            /// @param col Grid column.
            /// @return Slot, 0 when nobody is there.
            int slotAt(int row, int col) const;

        private:
            std::array<std::optional<Fighter>, kSlots + 1> m_fighters{};
    };

}

#endif
