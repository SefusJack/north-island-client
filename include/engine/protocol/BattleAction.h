#ifndef NORTH_ISLAND_PROTOCOL_BATTLE_ACTION_H
#define NORTH_ISLAND_PROTOCOL_BATTLE_ACTION_H

#include "protocol/BattleTarget.h"

#include <vector>

namespace protocol {

    /// @brief One action of a round (server 50/1).
    /// @details Original: RevAttackSkill.
    struct BattleAction {
        int row = 0;
        int col = 0;
        int skillId = 0;
        std::vector<BattleTarget> targets;
    };

}

#endif
