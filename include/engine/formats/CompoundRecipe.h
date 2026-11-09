#ifndef NORTH_ISLAND_FORMATS_COMPOUND_RECIPE_H
#define NORTH_ISLAND_FORMATS_COMPOUND_RECIPE_H

#include <array>
#include <cstddef>

namespace formats {

    /// @brief One recipe of data\Compound2.dat (TCompoundRec): what is made, where, from what and in how many minutes a batch.
    struct CompoundRecipe {
        /// @brief One material: an item and how many of it.
        struct Material {
            int itemId = 0;
            int count = 0;
        };

        static constexpr std::size_t kMaterials = 6;

        int productId = 0;
        int blueprintItemId = 0;
        int facilityId = 0;
        int productCount = 0;
        std::array<Material, kMaterials> materials{};
        int materialCount = 0;
        int makeMinutes = 0;

        /// @brief Gives a material place (numbered from 1).
        /// @param number Material place, 1..5.
        /// @return The material; throws when out of range.
        const Material& material(int number) const { return materials.at(static_cast<std::size_t>(number)); }
    };

}

#endif
