#ifndef NORTH_ISLAND_CLIENT_RULES_STORE_ITEM_TEXT_H
#define NORTH_ISLAND_CLIENT_RULES_STORE_ITEM_TEXT_H

#include "formats/ItemRecord.h"

#include <string>

namespace client {

    /// @brief Builds the text a store row shows for an item.
    /// @details Original: AddNpcStoreItemRow / AddPointStoreRow. Attributes 25 / 26 and 207..216 as "HP: +200" and the like, the shot resistance in per cent; in an equipment store (or the point store) the level it needs; else the description.
    /// @param item The item record.
    /// @param withLevel True in an equipment store or the point store (adds the needed level).
    /// @return The row text.
    std::string storeItemText(const formats::ItemRecord& item, bool withLevel);

}

#endif
