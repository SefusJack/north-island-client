#ifndef NORTH_ISLAND_CLIENT_RULES_ITEM_INFO_H
#define NORTH_ISLAND_CLIENT_RULES_ITEM_INFO_H

#include "formats/ItemTable.h"
#include "protocol/ItemInstance.h"

#include <string>
#include <vector>

namespace client {

    /// @brief The HP and SP the player's pools hold, shown on the special Bottles of HP / SP.
    struct BottlePools {
        std::int32_t hp = 0;
        std::int32_t sp = 0;
    };

    /// @brief Builds the lines of the item info popup.
    /// @details Original: TSe_PopInfoForm.ShowItemImageInfo (ShowThingItemInfo for an item lying in a house, which leaves the description whole).
    /// @param items The item table.
    /// @param item The item instance.
    /// @param wrapDescription Whether to wrap the description at word ends with a hyphen.
    /// @param pools What the player's HP and SP pools hold (for Bottles of kind 3 and 4).
    /// @return The popup lines.
    std::vector<std::string> describeItem(
        const formats::ItemTable& items,
        const protocol::ItemInstance& item,
        bool wrapDescription = true,
        const BottlePools& pools = {}
    );

    /// @brief The name a stall list shows, with the fusion level of worn gear.
    /// @details Original: RecvStallItemList.
    /// @param items The item table.
    /// @param item The item instance.
    /// @return The name.
    std::string stallItemName(const formats::ItemTable& items, const protocol::ItemInstance& item);

    /// @brief The one-line description beside a stall entry.
    /// @details Original: RecvStallItemList's DescText.
    /// @param items The item table.
    /// @param item The item instance.
    /// @param now The current time as a TDateTime.
    /// @return What it gives, level, uses, state, gem, time left, seal resistance or box, else the item description.
    std::string describeStallItem(const formats::ItemTable& items, const protocol::ItemInstance& item, double now);

    /// @brief The kind of a bottle item.
    /// @details Original: GetBottleKind (sub type 151).
    /// @param item The item record.
    /// @return 1 HP bottle, 2 SP bottle, 3 and 4 the special ones, 0 not a bottle.
    int bottleKind(const formats::ItemRecord& item);

    /// @brief The hours a worn item lasts.
    /// @details Original: GetItemUsePeriod.
    /// @param item The item record.
    /// @return Hours, 0 for ever.
    int usePeriod(const formats::ItemRecord& item);

    /// @brief Whether an item is mall gear (strengthened with Strong Scrolls, not forged).
    /// @details Original: IsMallEquipment.
    /// @param item The item record.
    /// @return True for mall gear.
    bool isMallEquipment(const formats::ItemRecord& item);

    /// @brief Whether an item is used a number of times.
    /// @details Original: IsMultiUseItem (sub types 130, 135, 140 with uses, and some named items).
    /// @param item The item record.
    /// @return True for a multi-use item.
    bool isMultiUse(const formats::ItemRecord& item);

    /// @brief The text that follows the name over an item's picture.
    /// @details Original: TSe_ItemImage.DrawHintBox; e.g. "(Remaining times:24Times)", "(Time:...)", "+3".
    /// @param record The item record.
    /// @param item The item instance.
    /// @return The suffix, empty for most items.
    std::string hintSuffix(const formats::ItemRecord& record, const protocol::ItemInstance& item);

}

#endif
