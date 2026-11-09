#ifndef NORTH_ISLAND_CLIENT_RULES_PIANO_SEAT_H
#define NORTH_ISLAND_CLIENT_RULES_PIANO_SEAT_H

#include "client/models/house/HomeFurniture.h"
#include "core/Point.h"
#include "formats/ItemTable.h"

namespace client {

    /// @brief Where a player sits at the Piano.
    /// @details Original: CJ_Menu.pas TPianoForm.CheckPianoSeats, DZ_Human.pas UseUtensil for IsPianoItem.
    struct PianoSeat {
        /// @brief Where the player stands and which way he faces (a direction 0..7 in the sitting group).
        struct Place {
            core::Point position;
            int direction = 0;
        };

        /// @brief Whether the cells along the front of a Piano are free of other pieces and inside the room.
        /// @details Original: CheckPianoSeats.
        /// @param furniture The house furniture.
        /// @param items The item table.
        /// @param furnIndex The Piano's furniture index.
        /// @return True when a player can sit.
        static bool isFree(const HomeFurniture& furniture, const formats::ItemTable& items, int furnIndex);

        /// @brief The seat of a Piano: 45 pixels to one side of the piece's floor point and 20 down.
        /// @details Original: UseUtensil. Facing 3 (FDirection slot 21) when the piece's picture is numbered even, else the other side facing 5 (slot 23).
        /// @param pieceWorld The piece's floor point in the world.
        /// @param evenPicture Whether the piece's picture number is even.
        /// @return The seat position and direction.
        static Place place(core::Point pieceWorld, bool evenPicture);
    };

}

#endif
