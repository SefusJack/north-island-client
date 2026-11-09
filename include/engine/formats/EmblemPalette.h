#ifndef NORTH_ISLAND_FORMATS_EMBLEM_PALETTE_H
#define NORTH_ISLAND_FORMATS_EMBLEM_PALETTE_H

#include <cstdint>

namespace formats {

    /// @brief Gives the RGB565 colour of a badge pixel value; 0 is no colour at all (nothing is drawn), as the empty value 15 has.
    /// @param index Badge pixel value, 0..15.
    /// @return The RGB565 colour, 0 for an index out of range.
    /// @details Original: EmblemPalette16 / GetEmblemPaletteColor.
    std::uint16_t emblemPaletteColor(int index);

}

#endif
