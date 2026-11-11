#include "client/subsystems/battle/BattleField.h"

namespace client {

    namespace {

        constexpr int kStepX = 75 / 3; // px each column moves sideways (towards the middle)
        constexpr int kDiagonalStepY = 53; // px each column moves down, before the gap
        constexpr int kGapY = 10; // extra px per column down
        constexpr int kBaseY = 310 - 44; // y of column 0 feet, px
        // x of each row's column 0 feet, px (index 0 unused; rows 1-2 left side, rows 3-4 right side)
        constexpr std::array<int, 5> kRowX{0, 160 + 50, 230 + 50, 590 - 50, 660 - 50};

    }

    core::Point BattleField::cellPosition(int row, int col) {
        const int x = row <= 2 ? kRowX.at(static_cast<std::size_t>(row)) - kStepX * col : kRowX.at(static_cast<std::size_t>(row)) + kStepX * col;
        return {x, (kDiagonalStepY + kGapY) * col + kBaseY};
    }

    void BattleField::clear() {
        for (auto& fighter : m_fighters) {
            fighter.reset();
        }
        effects = {};
    }

    int BattleField::freeSlot(bool ownPlayer) const {
        if (ownPlayer && !m_fighters[1]) {
            return 1;
        }
        for (int slot = 1; slot <= kSlots; ++slot) {
            if (!m_fighters[static_cast<std::size_t>(slot)]) {
                return slot;
            }
        }
        return 0;
    }

    void BattleField::place(int slot, Fighter fighter) {
        fighter.position = cellPosition(fighter.row, fighter.col);
        fighter.spawn = ++m_placed;
        m_fighters.at(static_cast<std::size_t>(slot)) = std::move(fighter);
    }

    std::optional<std::reference_wrapper<Fighter>> BattleField::fighter(int slot) {
        if (slot < 1 || slot > kSlots || !m_fighters[static_cast<std::size_t>(slot)]) {
            return std::nullopt;
        }
        return std::ref(*m_fighters[static_cast<std::size_t>(slot)]);
    }

    std::optional<std::reference_wrapper<const Fighter>> BattleField::fighter(int slot) const {
        if (slot < 1 || slot > kSlots || !m_fighters[static_cast<std::size_t>(slot)]) {
            return std::nullopt;
        }
        return std::cref(*m_fighters[static_cast<std::size_t>(slot)]);
    }

    int BattleField::slotAt(int row, int col) const {
        for (int slot = 1; slot <= kSlots; ++slot) {
            const auto& fighter = m_fighters[static_cast<std::size_t>(slot)];
            if (fighter && fighter->row == row && fighter->col == col) {
                return slot;
            }
        }
        return 0;
    }

}
