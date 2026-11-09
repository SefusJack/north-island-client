#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_THROWN_ITEM_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_THROWN_ITEM_H

namespace client {

    /// @brief An item a defeated fighter drops to a cell, flying there in an arc.
    /// @details Original: TFightMoveRec, packet 53/4.
    struct ThrownItem {
        int row = 0;
        int col = 0;
        int itemId = 0;
    };

}

#endif
