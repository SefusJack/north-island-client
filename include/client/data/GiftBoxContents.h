#ifndef NORTH_ISLAND_CLIENT_DATA_GIFT_BOX_CONTENTS_H
#define NORTH_ISLAND_CLIENT_DATA_GIFT_BOX_CONTENTS_H

#include "formats/ItemTable.h"

#include <functional>
#include <optional>
#include <vector>

namespace client {

    /// @brief What the mall's random packs hold.
    /// @details Original: se_mainUI.pas ShowGiftBoxContents with the tables of DZ_String.pas: the items and their counts, or for the 7 Colors Egg the pets (one item of the list each; a pet's count is 0).
    class GiftBoxContents {
        public:
            /// @brief One item of a box with its count.
            struct Entry {
                int itemId;
                int count;
            };

            /// @brief One random pack: its item id, whether it lists pets, and its entries.
            struct Box {
                int boxId;
                bool pets;
                std::vector<Entry> entries;
            };

            /// @brief Finds the list of a box.
            /// @param boxId Item id of the pack.
            /// @return The box, or nullopt for an item that has no list.
            static std::optional<std::reference_wrapper<const Box>> find(int boxId);

            /// @brief Tells whether an item is a random pack (Original: AC_Item.pas IsRandomPackItem).
            /// @param items The item table the item must exist in.
            /// @param itemId Item id to test.
            /// @return True for a random pack.
            static bool isRandomPack(const formats::ItemTable& items, int itemId);
    };

}

#endif
