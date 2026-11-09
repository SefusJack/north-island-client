#ifndef NORTH_ISLAND_CLIENT_DATA_FISHING_ROD_OFFSETS_H
#define NORTH_ISLAND_CLIENT_DATA_FISHING_ROD_OFFSETS_H

#include "core/Point.h"

namespace client {

    /// @brief Where the rod of a fisher is drawn against his feet.
    /// @details Original: RidePetOfsX and RidePetOfsY of CH_RidePet.pas, which DrawMovieRoleSprite adds for a sprite of the 009 bank while the player fishes; by body type (0..4) and the way he sits (the slots 46..53).
    class FishingRodOffsets {
        public:
            /// @brief Gives the rod's drawing offset for a body type and sitting slot.
            /// @param bodyType Body type (0..4).
            /// @param slot Animation slot (46..53).
            /// @return The offset, or (0, 0) when out of range.
            static core::Point offset(int bodyType, int slot);
    };

}

#endif
