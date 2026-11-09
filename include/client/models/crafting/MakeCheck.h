#ifndef NORTH_ISLAND_CLIENT_MODELS_CRAFTING_MAKE_CHECK_H
#define NORTH_ISLAND_CLIENT_MODELS_CRAFTING_MAKE_CHECK_H

#include "core/Bytes.h"
#include "formats/CompoundRecipe.h"

#include <array>

namespace client {

    /// @brief What the material check finds for a recipe made so many times.
    /// @details Original: CheckMakeMaterials (RE_Menu.pas). Holds how many of each material the player has, which are covered and the part of the 64/1 request that names what to use.
    struct MakeCheck {
        int quantity = 0;
        std::array<int, formats::CompoundRecipe::kMaterials> have{};
        std::array<int, formats::CompoundRecipe::kMaterials> need{};
        std::array<bool, formats::CompoundRecipe{}.materials.size()> enough{};
        bool ok = false;
        core::Bytes request;
    };

}

#endif
