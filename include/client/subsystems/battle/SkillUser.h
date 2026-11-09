#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_SKILL_USER_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_SKILL_USER_H

#include "formats/ItemRecord.h"
#include "formats/NpcInfo.h"

#include <array>
#include <functional>
#include <optional>

namespace client {

    /// @brief The own fighter about to command, as CheckCanUseSkill reads it.
    /// @details Kind (2 the player, 4 a pet), SP and worn weapon; for the player the role, worn clothes and learnt skills; for a pet its kind and three skill levels.
    struct SkillUser {
        int kind = 0;
        int sp = 0;
        int weaponId = 0;
        std::optional<formats::ItemRecord> weapon;
        int bodyType = 0;
        int head = 0;
        int clothesId = 0;
        std::function<int(int)> skillLevel;
        std::optional<formats::NpcInfo> npc;
        std::array<int, 3> petSkillLevels{};
    };

}

#endif
