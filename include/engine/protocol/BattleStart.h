#ifndef NORTH_ISLAND_PROTOCOL_BATTLE_START_H
#define NORTH_ISLAND_PROTOCOL_BATTLE_START_H

#include "protocol/FighterRecord.h"

#include <vector>

namespace protocol {

    /// @brief Server 11/250: u8 battle ground (SceneData 59000 + it), u8 the event's battle (0: none), then the fighters.
    /// @details Original: RevFightStart.
    struct BattleStart {
        int ground = 0;
        int eventBattle = 0;
        std::vector<FighterRecord> fighters;
    };

}

#endif
