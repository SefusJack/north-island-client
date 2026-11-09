#ifndef NORTH_ISLAND_CLIENT_MODELS_MALL_MALL_CART_LINE_H
#define NORTH_ISLAND_CLIENT_MODELS_MALL_MALL_CART_LINE_H

#include "protocol/MallRow.h"

namespace client {

    /// @brief A line of the mall's cart: the catalog row and how many of it.
    /// @details Original: FCart, FCartCounts.
    struct MallCartLine {
        protocol::MallRow row;
        int count = 1;
    };

}

#endif
