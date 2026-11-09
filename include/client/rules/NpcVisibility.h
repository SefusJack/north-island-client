#ifndef NORTH_ISLAND_CLIENT_RULES_NPC_VISIBILITY_H
#define NORTH_ISLAND_CLIENT_RULES_NPC_VISIBILITY_H

namespace client {

    /// @brief Whether this client keeps a map NPC hidden on a map.
    /// @details Original: ShouldHideMapNpc (region-specific cases of this build's region); the flee-kill game's NPC is hidden on its map while the game runs (BrManage.FFleeKillActive).
    /// @param mapId The map id.
    /// @param npcId The NPC kind id.
    /// @param fleeKillActive Whether the flee-kill game is running.
    /// @return True when hidden.
    bool shouldHideMapNpc(int mapId, int npcId, bool fleeKillActive);

    /// @brief Whether the flee-kill NPC is shown on its map while the game is not running.
    /// @details Original: ShouldShowMapNpc.
    /// @param mapId The map id.
    /// @param npcId The NPC kind id.
    /// @param fleeKillActive Whether the flee-kill game is running.
    /// @return True when shown.
    bool shouldShowMapNpc(int mapId, int npcId, bool fleeKillActive);

}

#endif
