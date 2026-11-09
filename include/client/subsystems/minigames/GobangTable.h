#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_GOBANG_TABLE_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_GOBANG_TABLE_H

#include <cstdint>

namespace client {

    /// @brief One of the sixteen tables of the Five in a Row hall (original LD_MiniGame.pas TGobangTable): seats, game state, turn and ante (0..4).
    struct GobangTable {
        std::int32_t player1 = 0;
        std::int32_t player2 = 0;
        bool occupied = false;
        bool movePending = false;
        int turn = 0;
        int ante = 0;
    };

}

#endif
