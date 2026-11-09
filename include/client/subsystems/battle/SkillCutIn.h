#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_SKILL_CUT_IN_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_SKILL_CUT_IN_H

#include <cstdint>
#include <optional>

namespace client {

    /// @brief Cut-in of the big skills: a streaked band across the screen with the caster's face flying along it.
    /// @details Original: effect layer S60000, FSpecEffectOn; runs for as many frames as the layer has.
    struct SkillCutIn {
        bool on = false;
        std::optional<std::uint64_t> startMs;
        int frame = 0;
        int lastFrame = 0;
    };

}

#endif
