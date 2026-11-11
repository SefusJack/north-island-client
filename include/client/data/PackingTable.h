#ifndef NORTH_ISLAND_CLIENT_DATA_PACKING_TABLE_H
#define NORTH_ISLAND_CLIENT_DATA_PACKING_TABLE_H

#include <optional>

namespace client {

    /// @brief The mall's strengthened gear (Original: PackingItemTable).
    /// @details Each of its 20 rows is a chain (the shop picture id, then up to 11 items, each one strengthening more than the one before); a piece of gear in a chain is strengthened with Strong Scrolls where the rest is forged.
    class PackingTable {
        public:
            /// @brief The place of an item in its chain: its strengthening level and whether the chain goes on.
            struct Step {
                int level = 0;
                bool canStrengthen = false;
            };

            /// @brief Finds the item's place in the chain of its shop picture.
            /// @param shopPicId The mall picture id naming the chain.
            /// @param itemId The item id to look for.
            /// @return The step, or nullopt when the item is not in the chain.
            static std::optional<Step> find(int shopPicId, int itemId);
    };

}

#endif
