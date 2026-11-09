#ifndef NORTH_ISLAND_PROTOCOL_BATTLE_VALUE_H
#define NORTH_ISLAND_PROTOCOL_BATTLE_VALUE_H

#include <cstdint>

namespace protocol {

    /// @brief A value an action changes on its target: attribute (25 HP, 26 SP, 221.. statuses), amount, and how it applies (1 taken away, 2 taken away by a critical hit, 0 given).
    struct BattleValue {
        int attribute = 0;
        std::int32_t amount = 0;
        int negative = 0;
    };

}

#endif
