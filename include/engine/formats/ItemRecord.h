#ifndef NORTH_ISLAND_FORMATS_ITEM_RECORD_H
#define NORTH_ISLAND_FORMATS_ITEM_RECORD_H

#include <array>
#include <cstdint>
#include <string>

namespace formats {

    /// @brief One item of data\Item.dat (the fields the client uses so far).
    struct ItemRecord {
        int id = 0;
        int type = 0;
        std::string name;
        int iconId = 0;
        std::array<int, 2> attributeKinds{};
        std::array<std::int32_t, 2> attributeValues{};
        int rank = 0;
        int subType = 0;
        int setId = 0;
        int usePeriod = 0;
        int maxUses = 0;
        int aliasItemId = 0;
        int shopPicId = 0;
        int requiredLevel = 0;
        int usableBy = 0;
        int petNpcId = 0;
        std::int32_t price = 0;
        std::int32_t sellPrice = 0;
        int flags = 0;
        int unlisted = 0;
        int combineKind = 0;
        int linkedSkillId = 0;
        std::string description;
        int cellWidth = 1;
        int cellHeight = 1;
        std::array<int, 4> lookIds{};
        int equipSlot = 0;
        std::array<std::array<std::int32_t, 4>, 4> colorPatterns{};
        int weaponKind = 0;
        int furnSizeX = 0;
        int furnSizeY = 0;
        int furnSizeZ = 0;
        int furnPicture0 = 0;
        int furnPicture1 = 0;
        int furnPlaceFlags = 0;
    };

}

#endif
