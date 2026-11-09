#ifndef NORTH_ISLAND_CLIENT_RULES_STALL_DROP_RULES_H
#define NORTH_ISLAND_CLIENT_RULES_STALL_DROP_RULES_H

#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "formats/ItemTable.h"

#include <optional>
#include <string>

namespace client {

    /// @brief Checks whether a bag item may be put on a shelf or into a vending machine of the own house.
    /// @details Original: Se_EquipUI.pas HandleItemDrop's checks, in the same order.
    class StallDropRules {
        public:
            /// @brief Why a drop is refused: the message and how long it shows.
            struct Refusal {
                std::string text;
                int durationMs = 0;
            };

            /// @brief Binds the rules to the item table, the inventory and the bag marks.
            /// @param items Item table used to look up flags and size.
            /// @param inventory Inventory holding the bag items.
            /// @param marks Bag lock marks.
            StallDropRules(
                const formats::ItemTable& items,
                const Inventory& inventory,
                const BagMarks& marks
            )
                : m_items(items),
                  m_inventory(inventory),
                  m_marks(marks) {}

            /// @brief Runs the drop checks for the item in a bag place.
            /// @param bagSlot Bag place of the item.
            /// @param owner True when the player owns the house.
            /// @param full True when every place of the piece already holds something.
            /// @param vending True for a vending machine rather than a shelf.
            /// @return The refusal, or empty when the item may be dropped.
            std::optional<Refusal> check(int bagSlot, bool owner, bool full, bool vending) const;

        private:
            const formats::ItemTable& m_items;
            const Inventory& m_inventory;
            const BagMarks& m_marks;
    };

}

#endif
