#ifndef NORTH_ISLAND_CLIENT_RULES_EQUIPPING_H
#define NORTH_ISLAND_CLIENT_RULES_EQUIPPING_H

#include "formats/ItemRecord.h"

#include <optional>
#include <string>

namespace client {

    /// @brief Tells why the own player may not put on an item.
    /// @details Original: TEquipManage.EquipBagItem.
    /// @param item The item.
    /// @param level The player's level.
    /// @param bodyType The player's body type.
    /// @param element The player's element.
    /// @return The message that says why not, or nothing when it may be worn.
    std::optional<std::string> equipRefusal(const formats::ItemRecord& item, int level, int bodyType, int element);

    /// @brief Tells whether the own player's body and element allow the item (made for no pet).
    /// @details Original: FUsableBy.
    /// @param item The item.
    /// @param bodyType The player's body type.
    /// @param element The player's element.
    /// @return True when the item may be worn.
    bool canWear(const formats::ItemRecord& item, int bodyType, int element);

    /// @brief Tells whether an item is worn at all (equipment slots 1..6).
    /// @details Original: IsEquipmentItem.
    /// @param item The item.
    /// @return True for equipment.
    bool isEquipment(const formats::ItemRecord& item);

}

#endif
