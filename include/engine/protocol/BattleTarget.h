#ifndef NORTH_ISLAND_PROTOCOL_BATTLE_TARGET_H
#define NORTH_ISLAND_PROTOCOL_BATTLE_TARGET_H

#include "protocol/BattleValue.h"

#include <vector>

namespace protocol {

    /// @brief Whom an action reaches: row, col, hit, guard (1 defence pose, 2 guard pose, 4 values at once) and the values.
    struct BattleTarget {
        int row = 0;
        int col = 0;
        bool hit = false;
        int guard = 0;
        std::vector<BattleValue> values;
    };

}

#endif
