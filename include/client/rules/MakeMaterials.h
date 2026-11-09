#ifndef NORTH_ISLAND_CLIENT_RULES_MAKE_MATERIALS_H
#define NORTH_ISLAND_CLIENT_RULES_MAKE_MATERIALS_H

#include "client/models/house/BookShelves.h"
#include "client/models/house/HomeFurniture.h"
#include "client/models/crafting/MakeCheck.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "formats/CompoundRecipe.h"

namespace client {

    /// @brief Counts what the bag and the house hold of a recipe's materials.
    /// @details Original: CheckMakeMaterials. Furniture counts when it stands alone; what the shelves, the gas can and the small gas hold is not built.
    class MakeMaterials {
        public:
            /// @brief Binds the check to the player's bag, house and shelves.
            /// @param inventory The bag.
            /// @param marks The bag's marks (items on sale or busy).
            /// @param furniture The house furniture.
            /// @param shelves The Bookcase and Desk shelves.
            MakeMaterials(
                const Inventory& inventory,
                const BagMarks& marks,
                const HomeFurniture& furniture,
                const BookShelves& shelves
            )
                : m_inventory(inventory),
                  m_marks(marks),
                  m_furniture(furniture),
                  m_shelves(shelves) {}

            /// @brief Where a blueprint was found: source (1 bag, 3 Bookcase, 4 Desk, 0 none) and place in it.
            struct BlueprintAt {
                int source = 0;
                int slot = 0;
            };

            /// @brief Checks a recipe made several times against the bag and house.
            /// @param recipe The recipe.
            /// @param quantity How many times it is made.
            /// @return The check: counts, whether enough, and the request body for the server.
            MakeCheck check(const formats::CompoundRecipe& recipe, int quantity) const;

            /// @brief The first bag place holding an item.
            /// @details Original: FindNextBagItemSlot.
            /// @param itemId The item id.
            /// @return The place, 0 for none.
            int bagSlotOf(int itemId) const;

            /// @brief Finds a recipe's blueprint: the bag, then the Bookcase, then the Desk (the bank is not built).
            /// @details Original: PickMakeThingRecipe.
            /// @param itemId The blueprint item id.
            /// @return Where it was found.
            BlueprintAt blueprintAt(int itemId) const;

            /// @brief Whether an item stacks.
            /// @details Original: IsStackableItem.
            /// @param itemId The item id.
            /// @return True when stackable.
            bool isStackable(int itemId) const { return m_inventory.isStackable(itemId); }

        private:
            const Inventory& m_inventory;
            const BagMarks& m_marks;
            const HomeFurniture& m_furniture;
            const BookShelves& m_shelves;
    };

}

#endif
