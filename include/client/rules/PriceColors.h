#ifndef NORTH_ISLAND_CLIENT_RULES_PRICE_COLORS_H
#define NORTH_ISLAND_CLIENT_RULES_PRICE_COLORS_H

#include "core/Color.h"

#include <cstddef>

namespace client {

    /// @brief The colour the game gives an amount of gold by its digit count (trades, stalls).
    /// @details Green from 4 digits, blue 5, magenta 6, red 7, else dark grey.
    /// @param digits The number of digits.
    /// @return The colour.
    core::Color priceColor(std::size_t digits);

}

#endif
