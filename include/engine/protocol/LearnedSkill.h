#ifndef NORTH_ISLAND_PROTOCOL_LEARNED_SKILL_H
#define NORTH_ISLAND_PROTOCOL_LEARNED_SKILL_H

#include <cstdint>

namespace protocol {

    /// @brief A skill the own player has, by its Skill.dat key.
    /// @details Original: TLearnedSkillRec.
    struct LearnedSkill {
        int key = 0;
        int level = 0;
        std::int32_t exp = 0;
    };

}

#endif
