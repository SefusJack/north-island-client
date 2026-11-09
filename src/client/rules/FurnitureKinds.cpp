#include "client/rules/FurnitureKinds.h"

namespace client {

    bool FurnitureKinds::isFloorItem(const formats::ItemTable& items, int itemId) {
        if (!items.find(itemId)) {
            return false;
        }
        // floor covering item ids: 39033..39036 and the single ids
        // 39033..39036 Tile; 39062 Wood Floor; 39067, 39068, 39131 Tile; 39133, 39151 Tatami; 39177 Night Sky Tile; 39178 Snow Ground Tile;
        // 39189 Tile; 39190, 39191 Love Tile; 39214 Snowy Tile
        return (itemId >= 39033 && itemId <= 39036) || itemId == 39062 || itemId == 39067 || itemId == 39068 || itemId == 39131 || itemId == 39133 ||
               itemId == 39151 || itemId == 39177 || itemId == 39178 || (itemId >= 39189 && itemId <= 39191) || itemId == 39214;
    }

    bool FurnitureKinds::isWallpaperItem(const formats::ItemTable& items, int itemId) {
        if (!items.find(itemId)) {
            return false;
        }
        switch (itemId) {
            case 39041: // Wallpaper
            case 39042: // Wallpaper
            case 39043: // Wallpaper
            case 39044: // Wallpaper
            case 39064: // Wallpaper
            case 39128: // Wallpaper
            case 39130: // Wallpaper
            case 39136: // Wallpaper
            case 39138: // Wallpaper
            case 39140: // Wallpaper
            case 39142: // Wallpaper
            case 39144: // Wallpaper
            case 39169: // Wallpaper
            case 39171: // Wallpaper
            case 39174: // Sky Wallpaper
            case 39176: // Snowflake Wallpaper
            case 39181: // Wallpaper
            case 39193: // Wallpaper
            case 39195: // Wallpaper
            case 39197: // Wallpaper
            case 39199: // Wallpaper
            case 39213: return true; // Snowy Wallpaper
            default: return false; // not a wallpaper
        }
    }

    bool FurnitureKinds::isLargeItem(const formats::ItemTable& items, int itemId) {
        const auto item = items.find(itemId);
        return item && !(item->cellWidth == 1 && item->cellHeight == 1);
    }

    int FurnitureKinds::bedKind(const formats::ItemTable& items, int itemId) {
        constexpr int kBedSubType = 108; // item sub type 108: beds (Hammock, Wooden Bed, Bunk Bed)
        const auto item = items.find(itemId);
        if (!item || item->subType != kBedSubType) {
            return 0;
        }
        switch (itemId) {
            case 39009: return 1; // Hammock holds 1 pet
            case 39012: return 2; // Wooden Bed holds 2 pets
            case 39124: return 4; // Bunk Bed holds 4 pets
            default: return 0; // not a bed
        }
    }

    int FurnitureKinds::maidKind(const formats::ItemTable& items, int itemId) {
        if (!items.find(itemId)) {
            return 0;
        }
        switch (itemId) {
            case 39066: return 1; // Maid Robot (no menu): kind 1
            case 39096: return 2; // Wit Maid: kind 2
            case 39117: return 3; // Wit Maid-Summer: kind 3
            case 39118: return 4; // Wit Maid-Rabbit: kind 4
            case 39119: return 5; // Wit Maid-Cat: kind 5
            case 39223: return 26; // Wit Maid-Pumpkin: kind 26
            case 39224: return 27; // Wit Maid-Succubus: kind 27
            case 39243: return 28; // Wit Maid-Qi Pao: kind 28
            default: return 0; // not a maid
        }
    }

    int FurnitureKinds::cupboardKind(const formats::ItemTable& items, int itemId) {
        constexpr int kCupboardSubType = 109; // item sub type 109: cupboards (Wardrobe, Cabinet)
        const auto item = items.find(itemId);
        if (!item || item->subType != kCupboardSubType) {
            return 0;
        }
        switch (itemId) {
            case 39018: return 1; // Wardrobe: kind 1
            case 39019: return 2; // Cabinet: kind 2
            default: return 0; // not a cupboard
        }
    }

    int FurnitureKinds::shelfKind(const formats::ItemTable& items, int itemId) {
        if (!items.find(itemId)) {
            return 0;
        }
        switch (itemId) {
            case 39059: return 1; // Shelf, the shelf an owner sells from: kind 1
            case 39086: return 3; // Food Shelf, display case: kind 3
            default: return 0; // not a shelf
        }
    }

    bool FurnitureKinds::isVendingMachine(const formats::ItemTable& items, int itemId) {
        constexpr int kVendingSubType = 0x6F; // item sub type 0x6F (111): vending machines (Coin Machine)
        const auto item = items.find(itemId);
        return item && item->subType == kVendingSubType && itemId == 39058; // 39058: Coin Machine
    }

    FurnitureKinds::Discard FurnitureKinds::checkHomeDiscard(const formats::ItemTable& items, int itemId) {
        const auto item = items.find(itemId);
        if (!item) {
            return Discard::Other;
        }
        const bool furniture = item->type == kFurnitureType;
        const bool instrument = item->type == kInstrumentType;
        const bool accessory = item->type == kAccessoryType;
        if (!furniture && !instrument && !accessory) {
            return Discard::Common;
        }
        if (furniture) {
            if (isWallpaperItem(items, itemId) || isFloorItem(items, itemId)) {
                return Discard::Other;
            }
        } else if (instrument) {
            if (!isLargeItem(items, itemId)) {
                return Discard::SmallInstrument;
            }
        } else if (!isLargeItem(items, itemId)) {
            return Discard::SmallAccessory;
        }
        if (item->furnSizeX == 0 || item->furnSizeY == 0 || item->furnSizeZ == 0) {
            return Discard::Other;
        }
        return Discard::Placeable;
    }

    int FurnitureKinds::wallLevel(const formats::ItemTable& items, int itemId) {
        if (!items.find(itemId)) {
            return 0;
        }
        switch (itemId) {
            case 39053: return 4; // Air Condition: wall piece at level 4
            case 39003: // Aspersory: wall piece at level 3
            case 39055: // Window: level 3
            case 39088: // Red Window: level 3
            case 39089: // Blue Window: level 3
            case 39152: // Gugu Clock: level 3
            case 39153: // Picture Frame: level 3
            case 39182: // Classic Light, West: level 3
            case 39183: // Classic Light, East: level 3
            case 39185: // Rose Love Wreath: level 3
            case 39186: // Rose Love Wreath: level 3
            case 39216: // Love Window: level 3
            case 39220: return 3; // Wall Painting: level 3
            case 39093: // Calligraphy: wall piece at level 2
            case 39125: // Calligraphy: level 2
            case 39146: // Spring Scrolls: level 2
            case 39147: // Spring Scrolls: level 2
            case 39148: // New Year Picture: level 2
            case 39149: // New Year Cracker: level 2
            case 39221: return 2; // Dumpling Deco: level 2
            case 39014: // Elevator: wall piece at level 1
            case 39030: // Storeroom: level 1
            case 39069: // Garage: level 1
            case 39087: // Decorative Dolphin: level 1
            case 39098: // French Window: level 1
            case 39107: // Cherry Door: level 1
            case 39108: // Beach Door: level 1
            case 39111: // Sweet Door: level 1
            case 39126: // Wall Furnace: level 1
            case 39172: // French Window: level 1
            case 39222: return 1; // Dragon Boat Deco: level 1
            default: return 0; // not a wall piece
        }
    }

}
