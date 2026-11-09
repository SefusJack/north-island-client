#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_MAP_PET_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_MAP_PET_H

#include "client/rules/PetWeapon.h"
#include "client/models/world/MapCharacter.h"
#include "core/Color.h"
#include "formats/NpcInfo.h"

#include <cstdint>
#include <string>

namespace client {

    /// @brief A player's fighting pet on the map: its kind's map sprite in its colours, walking after its owner.
    /// @details Original: TFollowNpc. A pet called back walks to its owner and is gone on arrival.
    struct MapPet {
        std::int32_t ownerId = 0;
        formats::NpcInfo info;
        std::string name;
        int tradable = 0;
        MapCharacter body;
        PetWeapon weapon;
        bool following = true;
        bool visible = true;
        bool walker = false;
        int walkMode = 0;
        bool maker = false;
        core::Point workAt;
        bool walkBy = false;
        int walkByEnd = 0;

        /// @brief The colour of the pet's name.
        /// @details Original: DrawHumInfo: cyan for a pet that may be renamed, orange for the others.
        /// @return The name colour.
        // RGB565 0x07FF is cyan (renamable pet), 0xFC00 is orange
        core::Color nameColor() const { return core::Color::fromRgb565(tradable == 1 ? 0x07FF : 0xFC00); }
    };

}

#endif
