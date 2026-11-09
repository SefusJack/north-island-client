#ifndef NORTH_ISLAND_CLIENT_RULES_EXP_BAG_AUTO_USE_H
#define NORTH_ISLAND_CLIENT_RULES_EXP_BAG_AUTO_USE_H

#include <cstdint>
#include <vector>

namespace client {

    /// @brief Decides when the experience bag drinks by itself and from which cell.
    /// @details Original: TExpBoxForm.CheckAutoUse (the conditions of the frame check and the cell it drinks from).
    class ExpBagAutoUse {
        public:
            /// @brief The state the original's frame check looks at.
            struct Conditions {
                int fightState = 0;
                bool sportActive = false;
                bool onPvpServer = false;
                bool autoDone = false;
                bool autoUse = false;
                bool doubleExpEnded = false;
                bool using_ = false;
            };

            /// @brief One cell of the experience bag.
            struct Cell {
                int itemId = 0;
                int count = 0;
            };

            /// @brief Tells whether all of the original's conditions for an automatic use hold.
            /// @param conditions The state to test.
            /// @return True when the bag is due to drink.
            static bool due(const Conditions& conditions) {
                return conditions.fightState == 0 && !conditions.sportActive && !conditions.onPvpServer && !conditions.autoDone && conditions.autoUse &&
                       conditions.doubleExpEnded && !conditions.using_;
            }

            /// @brief Finds the first of the cells 1 to 10 (the vector's index 1 to 10) holding something.
            /// @param cells The bag's cells, index 0 unused.
            /// @return The cell index, or 0 when all are empty.
            static int firstFilled(const std::vector<Cell>& cells) {
                for (std::size_t cell = 1; cell < cells.size() && cell <= 10; ++cell) { // the bag has 10 cells
                    if (cells[cell].itemId != 0 && cells[cell].count > 0) {
                        return static_cast<int>(cell);
                    }
                }
                return 0;
            }
    };

}

#endif
