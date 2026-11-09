#ifndef NORTH_ISLAND_CLIENT_RULES_FURNITURE_BONUS_H
#define NORTH_ISLAND_CLIENT_RULES_FURNITURE_BONUS_H

#include "client/models/house/HomeFurniture.h"
#include "formats/CompoundTable.h"

namespace client {

    /// @brief What the furniture of a house adds to manufacturing: shorter making time and bigger batches.
    /// @details Original: AC_Item.pas GetFurnitureBonusFlag, DZ_Human.pas CacheFurnitureFlags, RE_Menu.pas GetFurnitureMakeBonus.
    struct FurnitureBonus {
        /// @brief Gives the time-saving flag of a piece: 39053 15 per cent, 39007 2, 39045 3 and 37202 3 (flags 1, 2, 4 and 8).
        /// @details Original: GetFurnitureBonusFlag.
        /// @param itemId Item id of the piece.
        /// @return The flag, 0 for other pieces.
        static int flagOf(int itemId);

        /// @brief Gives one flag for each kind of piece in the house.
        /// @details Original: CacheFurnitureFlags.
        /// @param furniture The house's furniture.
        /// @return The combined flags.
        static int flags(const HomeFurniture& furniture);

        /// @brief Gives the minutes a batch takes with those flags, rounded up.
        /// @details Original: the rounding of LoadMakeThingTree and TRe_MakeThingInfoForm.DrawControl.
        /// @param baseMinutes Minutes without furniture.
        /// @param flags Flags from flags().
        /// @return The minutes.
        static int minutes(int baseMinutes, int flags);

        /// @brief Gives the products more a batch gives from the neighbours of the work platform.
        /// @details Original: GetFurnitureMakeBonus. By the product (mode 0) its recipe's facility, or the facility itself (mode 1).
        /// @param furniture The house's furniture.
        /// @param compounds The compound table.
        /// @param id The product id (mode 0) or the facility id (mode 1).
        /// @param mode 0 by product, 1 by facility.
        /// @return The bonus in percent.
        static int extraProducts(const HomeFurniture& furniture, const formats::CompoundTable& compounds, int id, int mode);

        /// @brief Finds the first piece of the item in the same room within the radius, counted in cells.
        /// @details Original: FindFurnitureNear.
        /// @param furniture The house's furniture.
        /// @param piece The piece to search around.
        /// @param itemId Item id to find.
        /// @param radius Distance in cells.
        /// @return The piece number, 0 for none.
        static int findNear(const HomeFurniture& furniture, int piece, int itemId, int radius);

        /// @brief Finds a piece of the item in any room; the retained pieces count unless settledOnly.
        /// @details Original: FindFurnitureById.
        /// @param furniture The house's furniture.
        /// @param itemId Item id to find.
        /// @param settledOnly Skip retained pieces.
        /// @return The piece number, 0 for none.
        static int findById(const HomeFurniture& furniture, int itemId, bool settledOnly = false);
    };

}

#endif
