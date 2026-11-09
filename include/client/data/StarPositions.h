#ifndef NORTH_ISLAND_CLIENT_DATA_STAR_POSITIONS_H
#define NORTH_ISLAND_CLIENT_DATA_STAR_POSITIONS_H

#include "core/Point.h"

namespace client {

    /// @brief Where the stars of the star book's sky stand.
    /// @details Original: StarLayoutPos1..28 of CH_Menu.pas.
    class StarPositions {
        public:
            /// @brief The place of a layout slot.
            /// @param slot Layout slot.
            /// @return Position, (-1, -1) for none.
            static core::Point layout(int slot);
    };

}

#endif
