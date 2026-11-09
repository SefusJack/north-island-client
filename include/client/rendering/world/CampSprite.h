#ifndef NORTH_ISLAND_CLIENT_RENDERING_WORLD_CAMP_SPRITE_H
#define NORTH_ISLAND_CLIENT_RENDERING_WORLD_CAMP_SPRITE_H

#include "client/models/world/Camp.h"
#include "formats/ItemTable.h"

#include <optional>
#include <string>

namespace client {

    /// @brief A sprite of the 009 banks: the bank name and the sprite id in it.
    struct CampSprite {
        std::string bank;
        int id = 0;
    };

    /// @brief Chooses what a tent looks like.
    /// @details Original: DrawCamp. Its own look (the tent item's sprite or its style's) shows for the owner and for everybody when the character information form says so (not on a Trojan war map, where only the owner sees it); the others see the plain sign (9010) or the style's next sprite. The later data (JmaLevel 4 and 5): the styles 9071 to 9093 are in the 009_c01 bank.
    /// @param camp The camp.
    /// @param ownLook True when the tent shows its own look.
    /// @param items Item table for the tent item's sprite.
    /// @return The sprite, none when the tent item is unknown.
    std::optional<CampSprite> campSprite(const Camp& camp, bool ownLook, const formats::ItemTable& items);

}

#endif
