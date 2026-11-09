#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_STALL_SIGN_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_STALL_SIGN_H

#include <string>

namespace client {

    /// @brief A stall a player keeps: what it sells, its name on the plate over the head and its picture at the feet.
    /// @details Original: OpenItemStall, OpenPetStall. Sign n is the left (odd) or right (even) one of pair (n + 1) / 2.
    struct StallSign {
        /// @brief What the stall sells.
        enum class Kind { Items, Pets, Furniture };

        Kind kind = Kind::Items;
        std::string name;
        int sign = 1;

        /// @brief Gives the name of the sign's picture.
        /// @details Original: FStallSignPics / FPetStallIconPics.
        /// @return The picture name.
        std::string pictureName() const {
            const std::string family = kind == Kind::Pets ? "Icon_PetStall_" : "Icon_ThingStall_";
            return family + (sign % 2 == 1 ? "L_" : "R_") + std::to_string((sign + 1) / 2);
        }

        /// @brief Tells whether the sign stands behind the figure (sign 3 of an item stall, or a pet stall's).
        /// @details Original: DrawBasePlayer.
        /// @return True when drawn behind.
        bool behindFigure() const { return kind == Kind::Pets || sign == 3; } // sign 3 of an item stall stands behind the figure
    };

}

#endif
