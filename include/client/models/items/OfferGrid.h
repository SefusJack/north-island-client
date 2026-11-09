#ifndef NORTH_ISLAND_CLIENT_MODELS_ITEMS_OFFER_GRID_H
#define NORTH_ISLAND_CLIENT_MODELS_ITEMS_OFFER_GRID_H

#include <optional>
#include <vector>

namespace client {

    /// @brief The cells of one side of a trade; an item goes into the first slot, along the rows, where all its cells are free, and owns them.
    /// @details Original: FGridCellOwner, FPartnerGridCells; 5 x 4 for items and pets, 5 x 10 for large items.
    class OfferGrid {
        public:
            static constexpr int kColumns = 5;
            static constexpr int kRows = 4;
            static constexpr int kSlots = kColumns * kRows;

            /// @brief Creates an empty grid.
            /// @param rows Number of rows.
            explicit OfferGrid(
                int rows = kRows
            )
                : m_rows(rows),
                  m_owners(static_cast<std::size_t>(kColumns * rows + 1), 0) {}

            /// @brief Finds the slot an item of that size takes and claims it.
            /// @details Original: FindOfferSpace.
            /// @param width Item width in cells.
            /// @param height Item height in cells.
            /// @return The slot, or nothing when it does not fit.
            std::optional<int> place(int width, int height);

            /// @brief Frees the cells an item at a slot covers.
            /// @param slot Slot of the item.
            /// @param width Item width in cells.
            /// @param height Item height in cells.
            void free(int slot, int width, int height);

            /// @brief Claims the cells an item at a given slot covers (the cells past the edge stay out).
            /// @details Original: MarkBagOccupancy.
            /// @param slot Slot of the item.
            /// @param width Item width in cells.
            /// @param height Item height in cells.
            void claim(int slot, int width, int height);

            /// @brief The slot whose item covers a cell.
            /// @param cell Cell number.
            /// @return The slot, or 0 for none.
            int ownerOf(int cell) const { return m_owners[static_cast<std::size_t>(cell)]; }

            /// @brief Frees every cell.
            void clear() { m_owners.assign(m_owners.size(), 0); }

        private:
            int m_rows;
            std::vector<int> m_owners;
    };

}

#endif