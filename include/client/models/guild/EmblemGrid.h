#ifndef NORTH_ISLAND_CLIENT_MODELS_GUILD_EMBLEM_GRID_H
#define NORTH_ISLAND_CLIENT_MODELS_GUILD_EMBLEM_GRID_H

#include "formats/EmblemBlock.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief A badge being drawn: the palette index of each of the 24 x 24 cells, [x][y].
    /// @details Original: TEmblemGrid.
    struct EmblemGrid {
        static constexpr int kSide = formats::EmblemBlock::kSide;

        std::array<std::array<std::uint8_t, kSide>, kSide> cells;

        /// @brief Creates a grid with all cells empty.
        EmblemGrid();

        /// @brief Reads a grid from a badge block.
        /// @details Original: DecodeEmblemGrid.
        /// @param block The badge block.
        /// @return The grid.
        static EmblemGrid decode(const formats::EmblemBlock& block);

        /// @brief Writes the grid to a badge block.
        /// @details Original: EncodeEmblemGrid.
        /// @return The badge block.
        formats::EmblemBlock encode() const;

        /// @brief Compares the cells of two grids.
        /// @param other The other grid.
        /// @return True when all cells are equal.
        bool operator==(const EmblemGrid& other) const { return cells == other.cells; }
    };

}

#endif
