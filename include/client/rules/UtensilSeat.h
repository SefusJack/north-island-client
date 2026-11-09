#ifndef NORTH_ISLAND_CLIENT_RULES_UTENSIL_SEAT_H
#define NORTH_ISLAND_CLIENT_RULES_UTENSIL_SEAT_H

#include "core/Point.h"
#include "formats/ItemTable.h"

#include <optional>

namespace client {

    /// @brief The furniture pieces a player sits on and where they put him.
    /// @details Original: AC_Item.pas GetFurnitureSeatCount, GetSitFurnitureKind, IsToiletItem, IsItem39218; DZ_Human.pas UseUtensil. The Piano, the Treadmill and the Bicep Exerciser seat their players by other rules.
    struct UtensilSeat {
        /// @brief Where a seated player stands and which way he faces.
        /// @details The direction is a slot 0..7 of FDirection's 49, 51, 21 and 23 in the action group; group is PlayHumanAction's group; holdFirstFrame is FAnimStopFrame := 0.
        struct Place {
            core::Point position;
            int direction = 0;
            int group = 46; // action group 46: sitting
            bool holdFirstFrame = false;
        };

        /// @brief Gives the number of seats of a piece (Original: GetFurnitureSeatCount).
        /// @param items Item table.
        /// @param itemId Id of the piece.
        /// @return 1 the Sofa (39046), 2 the Great Sofa (39156), 3 the Cannon Toy (39219), 0 any other piece.
        static int seatCount(const formats::ItemTable& items, int itemId);

        /// @brief Gives the sit kind of a piece (Original: GetSitFurnitureKind).
        /// @param itemId Id of the piece.
        /// @return 1 the Cushion (39116), 2 the White Sofa (39155), 3 the Red and Blue Carpets (39079, 39080), 0 any other piece.
        static int sitKind(int itemId);

        /// @brief Tells whether a piece is the Toilet Bowl (39060; Original: IsToiletItem).
        /// @param items Item table.
        /// @param itemId Id of the piece.
        /// @return True for the Toilet Bowl.
        static bool isToilet(const formats::ItemTable& items, int itemId);

        /// @brief Tells whether a piece is the Lovely Octopus (39218), which hides while a player sits in its place (Original: IsItem39218).
        /// @param items Item table.
        /// @param itemId Id of the piece.
        /// @return True for the Lovely Octopus.
        static bool isOctopus(const formats::ItemTable& items, int itemId);

        /// @brief Tells whether UseUtensil seats a player at the piece.
        /// @param items Item table.
        /// @param itemId Id of the piece.
        /// @return True for a seat piece.
        static bool isSeat(const formats::ItemTable& items, int itemId);

        /// @brief Gives the place of a seat at a piece (Original: UseUtensil).
        /// @details The picture's number being even or odd gives the piece's facing.
        /// @param items Item table.
        /// @param itemId Id of the piece.
        /// @param seat Seat number (1 or 2 on the Great Sofa).
        /// @param evenPicture True when the picture's number is even.
        /// @param world Floor point of the piece.
        /// @return The place, or nothing for a piece that is none of these.
        static std::optional<Place> place(
            const formats::ItemTable& items,
            int itemId,
            int seat,
            bool evenPicture,
            core::Point world
        );
    };

}

#endif
