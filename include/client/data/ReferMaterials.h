#ifndef NORTH_ISLAND_CLIENT_DATA_REFER_MATERIALS_H
#define NORTH_ISLAND_CLIENT_DATA_REFER_MATERIALS_H

#include <optional>
#include <string>

namespace client {

    /// @brief The materials the Magnifying Glass names: a code 1 to 68 is a name and the picture of its icon.
    /// @details Original: CJ_ReferToMaterial.pas MaterialNamePics, GetReferMaterialInfo.
    class ReferMaterials {
        public:
            /// @brief One material: its name and the name of its icon's picture.
            struct Material {
                std::string name;
                std::string picture;
            };

            static constexpr int kMostCode = 68; // highest material code (codes 1..68)

            /// @brief Looks up a material by code.
            /// @param code Material code (1..68).
            /// @return The material, or nullopt for 0 and for a code past 68.
            static std::optional<Material> of(int code);
    };

}

#endif
