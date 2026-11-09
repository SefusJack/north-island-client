#ifndef NORTH_ISLAND_CLIENT_RENDERING_WORLD_NPC_HOVER_LIGHT_H
#define NORTH_ISLAND_CLIENT_RENDERING_WORLD_NPC_HOVER_LIGHT_H

#include <cstdint>

namespace client {

    /// @brief The hover light of map NPCs: one with the mouse over one of its drawn pixels is drawn once more added to the screen at 140 / 255.
    /// @details Original: DrawMovieRoleSprite. Not for the furniture NPCs (IsFurnitureNpcId). That includes the signs and the other scene objects, which are NPCs of type 10.
    struct NpcHoverLight {
        static constexpr std::uint8_t kStrength = 140; // add strength out of 255

        /// @brief Tells whether an NPC gets the hover light.
        /// @param npcId NPC id.
        /// @return False for the furniture NPCs, true otherwise.
        static bool applies(int npcId);
    };

}

#endif
