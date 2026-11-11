#ifndef NORTH_ISLAND_CLIENT_RULES_SKILL_USE_H
#define NORTH_ISLAND_CLIENT_RULES_SKILL_USE_H

#include "client/subsystems/battle/SkillUser.h"
#include "formats/MotionTable.h"
#include "formats/SkillTable.h"

#include <optional>
#include <string>
#include <utility>

namespace client {

    /// @brief Checks whether a fighter may use a skill in battle.
    /// @details Original: CheckCanUseSkill. Results: 0 it may; 1 the weapon does not suit it; 2 not enough SP; 252 / 253 no such skill or motion; 255 not learnt or not its own.
    /// @param skills The skill table.
    /// @param motions The motion table.
    /// @param user The fighter.
    /// @param skillId The skill asked for; becomes the skill as it is sent (the plain attack becomes the weapon's own attack, a pet's its fixed skill), and 0 when it may not be used.
    /// @return The result code.
    int checkCanUseSkill(const formats::SkillTable& skills, const formats::MotionTable& motions, const SkillUser& user, int& skillId);

    /// @brief The message CheckCanUseSkill shows for a result.
    /// @param skills The skill table.
    /// @param result The result of checkCanUseSkill.
    /// @param skillId The skill asked for.
    /// @return The text and how long it shows in ms; none for 0.
    std::optional<std::pair<std::string, int>> skillUseMessage(const formats::SkillTable& skills, int result, int skillId);

    /// @brief The attack a weapon kind gives.
    /// @details Original: GetWeaponAttackSkill.
    /// @param weaponKind The weapon kind.
    /// @param npcVariant 0 a player, 1 a pet's or a shape's.
    /// @return The skill id, 0 for none.
    int weaponAttackSkill(int weaponKind, int npcVariant);

    /// @brief Whether the battle skill list shows a skill (a learnt skill, category 1..7 with a key).
    /// @details Original: IsBattleListSkill.
    /// @param skills The skill table.
    /// @param skillId The skill id.
    /// @return True when shown.
    bool isBattleListSkill(const formats::SkillTable& skills, int skillId);

}

#endif
