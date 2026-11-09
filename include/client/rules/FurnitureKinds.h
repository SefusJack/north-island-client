#ifndef NORTH_ISLAND_CLIENT_RULES_FURNITURE_KINDS_H
#define NORTH_ISLAND_CLIENT_RULES_FURNITURE_KINDS_H

#include "formats/ItemTable.h"

namespace client {

    /// @brief What an item is to a house: floor, wallpaper, furniture, instrument, accessory, large, and the special pieces.
    /// @details Original: AC_Item.pas IsFloorItem, IsWallpaperItem, IsFurnitureItem, IsInstrumentItem, IsAccessoryItem, IsLargeItem, CheckHomeDiscard, GetWallFurnitureZ.
    struct FurnitureKinds {
        static constexpr int kInstrumentType = 29; // item type 29: instruments (e.g. Assembly Basin, Auto Loom, Elevator)
        static constexpr int kFurnitureType = 30; // item type 30: furniture (e.g. ATM, Bookcase, Shelf)
        static constexpr int kAccessoryType = 28; // item type 28: house accessories / tools (e.g. AC Generator)
        static constexpr int kHoldsPieces = 1; // furnPlaceFlags bit 1: other pieces can be placed on it
        static constexpr int kLaysOnPiece = 2; // furnPlaceFlags bit 2: is placed on another piece
        static constexpr int kOnWall = 4; // furnPlaceFlags bit 4: leans on the wall / pillars

        /// @brief The answers of CheckHomeDiscard.
        /// @details Original: CheckHomeDiscard: 0 a piece that can be put down, 1 not furniture at all, 2 an instrument too small to stand, 3 an accessory too small; anything else (255) a floor or wall item or one of no cells.
        enum class Discard { Placeable = 0, Common = 1, SmallInstrument = 2, SmallAccessory = 3, Other = 255 }; // CheckHomeDiscard result codes

        /// @brief Tells whether an item is a floor covering.
        /// @details Original: IsFloorItem.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return True for a floor item.
        static bool isFloorItem(const formats::ItemTable& items, int itemId);

        /// @brief Tells whether an item is a wallpaper.
        /// @details Original: IsWallpaperItem.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return True for a wallpaper.
        static bool isWallpaperItem(const formats::ItemTable& items, int itemId);

        /// @brief Tells whether an item takes more than one cell.
        /// @details Original: IsLargeItem.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return True when larger than 1 x 1 cells.
        static bool isLargeItem(const formats::ItemTable& items, int itemId);

        /// @brief Tells whether an item can be put down in a house.
        /// @details Original: CheckHomeDiscard.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return The answer of CheckHomeDiscard.
        static Discard checkHomeDiscard(const formats::ItemTable& items, int itemId);

        /// @brief Gives how many pets a bed holds (the Hammock 39009: 1, the Wooden Bed 39012: 2, the Bunk Bed 39124: 4).
        /// @details Original: GetBedKind.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return The pet count, 0 for any other piece.
        static int bedKind(const formats::ItemTable& items, int itemId);

        /// @brief Gives the kind of a cupboard: 1 the Wardrobe (39018, five places), 2 the Cabinet (39019, ten).
        /// @details Original: GetCupboardKind.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return The kind, 0 for any other piece.
        static int cupboardKind(const formats::ItemTable& items, int itemId);

        /// @brief Gives the kind of a shelf: 1 the shelf an owner sells from (item 39059), 3 the display case (39086).
        /// @details Original: GetShelfKind.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return The kind, 0 for any other piece.
        static int shelfKind(const formats::ItemTable& items, int itemId);

        /// @brief Tells whether an item is the vending machine (item 39058).
        /// @details Original: IsVendingMachineItem.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return True for the vending machine.
        static bool isVendingMachine(const formats::ItemTable& items, int itemId);

        /// @brief Tells whether an item is the Fortune Egg machine (item 0x9891).
        /// @details Original: IsFortuneEggItem.
        /// @param itemId Item id.
        /// @return True for the Fortune Egg machine.
        static bool isFortuneEgg(int itemId) { return itemId == 0x9891; } // Fortune Egg machine item id

        /// @brief Tells whether an item is the Wire Game Machine (item 39094).
        /// @details Original: IsTvGameConsoleItem.
        /// @param itemId Item id.
        /// @return True for the Wire Game Machine.
        static bool isTvGameConsole(int itemId) { return itemId == 39094; } // Wire Game Machine item id

        /// @brief Tells whether an item is the Piano (item 39110).
        /// @details Original: IsPianoItem.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return True for the Piano.
        static bool isPiano(const formats::ItemTable& items, int itemId) { return itemId == 39110 && items.find(itemId).has_value(); } // Piano item id

        /// @brief Tells whether an item is the Bookcase (item 39054).
        /// @details Original: IsBookcaseItem.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return True for the Bookcase.
        static bool isBookcase(const formats::ItemTable& items, int itemId) {
            return itemId == 39054 && items.find(itemId).has_value();
        } // Bookcase item id (39054: Bookcase)

        /// @brief Tells whether an item is the Desk (item 39020).
        /// @details Original: IsBookDeskItem.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return True for the Desk.
        static bool isBookDesk(const formats::ItemTable& items, int itemId) { return itemId == 39020 && items.find(itemId).has_value(); } // Desk item id

        /// @brief Tells whether an item is a Picture Frame or Good Picture Frame (39153, 39154), whose click shows the owner's portrait.
        /// @param itemId Item id.
        /// @return True for a picture frame.
        static bool isPictureFrame(int itemId) { return itemId == 39153 || itemId == 39154; } // Picture Frame and Good Picture Frame item ids

        /// @brief Tells whether an item is the Doorplate (item 39056).
        /// @details Original: IsDoorplateItem.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return True for the Doorplate.
        static bool isDoorplate(const formats::ItemTable& items, int itemId) {
            return itemId == 39056 && items.find(itemId).has_value();
        } // Doorplate item id (39056: Doorplate)

        /// @brief Tells whether an item is the Cracker (item 39149).
        /// @details Original: IsNewYearCrackerItem.
        /// @param itemId Item id.
        /// @return True for the Cracker.
        static bool isNewYearCracker(int itemId) { return itemId == 39149; } // Cracker item id

        /// @brief Tells whether an item is the Gugu Clock (item 39152).
        /// @details Original: IsGuguClockItem.
        /// @param itemId Item id.
        /// @return True for the Gugu Clock.
        static bool isGuguClock(int itemId) { return itemId == 39152; } // Gugu Clock item id

        /// @brief Tells whether an item is the Elevator (item 39014) or the Stairs (item 39031).
        /// @details Original: IsStairsItem.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return True for the Elevator or the Stairs.
        static bool isStairs(const formats::ItemTable& items, int itemId) {
            return (itemId == 39014 || itemId == 39031) && items.find(itemId).has_value();
        } // Elevator and Stairs item ids (39014: Elevator, 39031: Stairs)

        /// @brief Tells whether an item is the Elevator (item 39014, which has an animation).
        /// @param itemId Item id.
        /// @return True for the Elevator.
        static bool isElevator(int itemId) { return itemId == 39014; } // Elevator item id

        /// @brief Tells whether an item is the Fish Tank (item 39099).
        /// @details Original: IsFishBoxItem.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return True for the Fish Tank.
        static bool isFishBox(const formats::ItemTable& items, int itemId) {
            return itemId == 39099 && items.find(itemId).has_value();
        } // Fish Tank item id (39099: Fish Urn in the item table)

        /// @brief Gives the kind of a maid: 1 the plain maid 39066 (no menu), 2 the Wit Maid 39096 and 3, 4, 5, 26, 27, 28 her costumes 39117, 39118, 39119, 39223, 39224, 39243.
        /// @details Original: GetMaidKind.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return The kind, 0 for the others.
        static int maidKind(const formats::ItemTable& items, int itemId);

        /// @brief Tells whether an item is the Blackboard (item 39073) and Item.dat gives it a rank.
        /// @details Original: IsBlackboardItem.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return True for the Blackboard.
        static bool isBlackboard(const formats::ItemTable& items, int itemId) {
            const auto item = itemId == 39073 ? items.find(itemId) : std::nullopt; // Blackboard item id
            return item && item->rank != 0;
        }

        /// @brief Gives the level a wall piece hangs at.
        /// @details Original: GetWallFurnitureZ.
        /// @param items The item table.
        /// @param itemId Item id.
        /// @return The level, 0 when it is not a wall piece.
        static int wallLevel(const formats::ItemTable& items, int itemId);
    };

}

#endif
