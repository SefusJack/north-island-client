#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_TARGET_EFFECT_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_TARGET_EFFECT_H

#include <cstdint>
#include <vector>

namespace client {

    /// @brief What an action does to one fighter it reaches: cell, hit and guard, and the values with what is left of each over the hits.
    /// @details Original: THumEffectRec, queued on the acting fighter by AddHumEffects.
    struct TargetEffect {
        /// @brief One attribute value of the effect with its total, remainder and this hit's part.
        struct Value {
            int attribute = 0;
            std::int32_t total = 0;
            std::int32_t left = 0;
            std::int32_t hit = 0;
            int negative = 0;
        };

        int skillId = 0;
        int row = 0;
        int col = 0;
        bool hit = false;
        int guard = 0;
        std::vector<Value> values;
        int hitsDone = 0;
    };

}

#endif
