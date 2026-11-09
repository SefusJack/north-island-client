#ifndef NORTH_ISLAND_PROTOCOL_FIGHTER_CAST_H
#define NORTH_ISLAND_PROTOCOL_FIGHTER_CAST_H

#include <cstdint>

namespace protocol {

    /// @brief Server 50/3: a skill cast whose target takes an attribute change when the caster is back.
    /// @details Original: RecvFightSkillCast. The kind is the skill's status effect: 241 and 242 change the target's attribute.
    struct FighterCast {
        int casterRow = 0;
        int casterCol = 0;
        int targetRow = 0;
        int targetCol = 0;
        int skillId = 0;
        int kind = 0;
        int attribute = 0;
        int negative = 0;
        std::int32_t amount = 0;
    };

}

#endif
