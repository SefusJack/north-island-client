#ifndef NORTH_ISLAND_FORMATS_COMPOUND_TABLE_H
#define NORTH_ISLAND_FORMATS_COMPOUND_TABLE_H

#include "core/Bytes.h"
#include "formats/CompoundRecipe.h"

#include <optional>
#include <vector>

namespace formats {

    /// @brief The recipe table read from data\Compound2.dat.
    /// @details Original: data\Compound2.dat, 65-byte records, the first one a header (its product id is 6). Words are stored (value + 3) XOR 0xFBBC reversed, i.e. (stored XOR key) - 3, bytes (stored XOR 211) - 3. The recipes are counted from 1.
    class CompoundTable {
        public:
            static constexpr std::size_t kRecordSize = 65; // bytes per recipe record in Compound2.dat

            /// @brief Decodes the file contents.
            /// @param data Contents of Compound2.dat.
            /// @return The table, nothing when the size or header is wrong.
            static std::optional<CompoundTable> parse(const core::Bytes& data);

            /// @brief Gives the number of recipes (the header not counted).
            /// @return The recipe count.
            int size() const { return static_cast<int>(m_recipes.size()) - 1; }

            /// @brief Gives a recipe.
            /// @param index Recipe number, 1..size().
            /// @return The recipe, an empty one for anything else.
            const CompoundRecipe& at(int index) const;

            /// @brief Finds the recipe that makes a product.
            /// @param productId Product item id.
            /// @return The recipe number, 0 for none.
            /// @details Original: GetCompoundIndex.
            int indexOfProduct(int productId) const;

            /// @brief Finds the product of the last recipe that names a blueprint.
            /// @param blueprintItemId Blueprint item id.
            /// @return The product id, 0 for none.
            /// @details Original: GetBlueprintProductId.
            int productOfBlueprint(int blueprintItemId) const;

            /// @brief Finds the first recipe from a start index on that is made at a piece of furniture.
            /// @param facilityId Furniture id.
            /// @param startIndex Recipe number to start at.
            /// @return The recipe number, 0 for none.
            /// @details Original: FindCompoundByFacility.
            int findByFacility(int facilityId, int startIndex) const;

            /// @brief Sets where a recipe is made now; nothing happens for an index that is no recipe.
            /// @param index Recipe number.
            /// @param facilityId Furniture id (0: nowhere).
            /// @details Original: RecvCompoundFacilityUpdate.
            void setFacility(int index, int facilityId);

        private:
            std::vector<CompoundRecipe> m_recipes;
    };

}

#endif
