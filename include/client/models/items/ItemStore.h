#ifndef NORTH_ISLAND_CLIENT_MODELS_ITEMS_ITEM_STORE_H
#define NORTH_ISLAND_CLIENT_MODELS_ITEMS_ITEM_STORE_H

#include "protocol/BagItemRecord.h"
#include "protocol/ItemInstance.h"

#include <vector>

namespace client {

    /// @brief A row of places holding one item each, stacks of up to 50.
    /// @details Original: TPlayer.FItemStore2 (the ten places of the sale store); RecvItemStore2List puts records in their places, RecvItemStore2Add and AddToItemStore2 find them one, RecvItemStore2Clear empties one.
    class ItemStore {
        public:
            /// @brief Creates an empty store.
            /// @param slots Number of places.
            explicit ItemStore(
                int slots
            )
                : m_items(static_cast<std::size_t>(slots) + 1) {}

            /// @brief Number of places.
            /// @return The place count.
            int slots() const { return static_cast<int>(m_items.size()) - 1; }

            /// @brief The item in a place.
            /// @param slot Place 1..slots().
            /// @return The item.
            const protocol::ItemInstance& item(int slot) const { return m_items.at(static_cast<std::size_t>(slot)); }

            /// @brief Puts a record into its place.
            /// @details A 0 place means: onto a stack of the same item not yet full when the item stacks, else the first empty place, on to the next while some of the count is left. Original: AddThingItem puts at most 50 in a place.
            /// @param record The record to add.
            /// @param stackable Whether the item stacks.
            void add(const protocol::BagItemRecord& record, bool stackable);

            /// @brief Empties a place.
            /// @param slot Place.
            void clear(int slot);

        private:
            std::vector<protocol::ItemInstance> m_items;
    };

}

#endif
