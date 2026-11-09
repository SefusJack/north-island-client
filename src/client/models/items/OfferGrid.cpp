#include "client/models/items/OfferGrid.h"

namespace client {

    std::optional<int> OfferGrid::place(int width, int height) {
        for (int slot = 1; slot <= kColumns * m_rows; ++slot) {
            const int column = (slot - 1) % kColumns;
            const int row = (slot - 1) / kColumns;
            bool fits = column + width <= kColumns && row + height <= m_rows;
            for (int x = column; fits && x < column + width; ++x) {
                for (int y = row; fits && y < row + height; ++y) {
                    fits = m_owners[static_cast<std::size_t>(y * kColumns + x + 1)] == 0;
                }
            }
            if (!fits) {
                continue;
            }
            for (int x = column; x < column + width; ++x) {
                for (int y = row; y < row + height; ++y) {
                    m_owners[static_cast<std::size_t>(y * kColumns + x + 1)] = slot;
                }
            }
            return slot;
        }
        return std::nullopt;
    }

    void OfferGrid::claim(int slot, int width, int height) {
        const int column = (slot - 1) % kColumns;
        const int row = (slot - 1) / kColumns;
        for (int x = column; x < column + width && x < kColumns; ++x) {
            for (int y = row; y < row + height && y < m_rows; ++y) {
                m_owners[static_cast<std::size_t>(y * kColumns + x + 1)] = slot;
            }
        }
    }

    void OfferGrid::free(int slot, int width, int height) {
        const int column = (slot - 1) % kColumns;
        const int row = (slot - 1) / kColumns;
        for (int x = column; x < column + width && x < kColumns; ++x) {
            for (int y = row; y < row + height && y < m_rows; ++y) {
                m_owners[static_cast<std::size_t>(y * kColumns + x + 1)] = 0;
            }
        }
    }

}
