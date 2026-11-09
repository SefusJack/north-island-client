#ifndef NORTH_ISLAND_CLIENT_RULES_THROW_ITEMS_H
#define NORTH_ISLAND_CLIENT_RULES_THROW_ITEMS_H

namespace client {

    /// @brief Classifies the items a click on the map throws.
    class ThrowItems {
        public:
            /// @brief Gives the throw kind of an item (Original: GetThrowItemKind).
            /// @param itemId Id of the item.
            /// @return 1, 2 or 3 for a thrown item, 0 when not a thrown item.
            static int kindOf(int itemId);
    };

}

#endif
