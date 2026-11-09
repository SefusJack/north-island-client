#ifndef NORTH_ISLAND_CLIENT_RULES_ITEM_USING_H
#define NORTH_ISLAND_CLIENT_RULES_ITEM_USING_H

#include "formats/ItemRecord.h"

#include <cstdint>
#include <string>

namespace client {

    /// @brief The player or pet an item is to be used on.
    struct UseTarget {
        bool pet = false;
        std::int32_t hp = 0;
        std::int32_t maxHp = 0;
        int sp = 0;
        int maxSp = 0;
        std::int64_t exp = 0;
        int reborn = 0;
        int tradable = 1;
        bool mercenary = false;
    };

    /// @brief Why an item may not be used: the message and how long it shows.
    struct UseRefusal {
        std::string text;
        int ms = 0;
    };

    /// @brief Checks whether an item may be used on a target.
    /// @details Original: the target checks of RequestUseBagItem.
    /// @param item The item record.
    /// @param target The player or pet it is used on.
    /// @param fighting Whether the player is in a fight.
    /// @return The refusal; empty text when the item may be used.
    UseRefusal useRefusal(const formats::ItemRecord& item, const UseTarget& target, bool fighting);

}

#endif
