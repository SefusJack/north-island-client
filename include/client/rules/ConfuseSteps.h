#ifndef NORTH_ISLAND_CLIENT_RULES_CONFUSE_STEPS_H
#define NORTH_ISLAND_CLIENT_RULES_CONFUSE_STEPS_H

namespace client {

    /// @brief Gives the step of a stone mounted in a piece of gear: 1 to 16 for the magic crystal stones 61063 to 61078 in order.
    /// @details Original: NonTransferableItemTable (AC_Item.pas:280).
    /// @param stoneItemId Item id of the stone.
    /// @return The step, 0 for any other item.
    int confuseStoneStep(int stoneItemId);

    /// @brief Gives the level of a Chaos Crystal: 1 for the Chaos Crystal 61062, 2 for the Shadow Chaos Crystal 61079, 3 for the Pure Chaos Crystal 61080.
    /// @details Original: GetConfuseStoneLevel.
    /// @param itemId Item id.
    /// @return The level, 0 for any other item.
    int confuseCrystalLevel(int itemId);

}

#endif
