#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_STATUS_RULES_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_STATUS_RULES_H

#include "client/subsystems/battle/FighterStatuses.h"
#include "formats/SkillTable.h"

#include <cstdint>

namespace client {

    /// @brief What a status value of an action did.
    /// @details Original: ApplyEffectToFighter's attributes 221..236.
    enum class StatusValueResult { NotStatus, Applied, BecomesHp };

    /// @brief Puts the skill among the status icons.
    /// @details Original: AddFighterStatus.
    /// @param statuses Fighter's statuses.
    /// @param skillId Skill whose icon is added.
    /// @param special 1 or 2: the special places.
    void addStatusIcon(FighterStatuses& statuses, int skillId, int special = 0);

    /// @brief Removes the skill's icon; the colour comes from the first icon left that gives one.
    /// @details Original: RemoveFighterStatus.
    /// @param statuses Fighter's statuses.
    /// @param skills Skill table.
    /// @param skillId Skill whose icon is removed.
    /// @param special 1 or 2: the special places.
    void removeStatusIcon(FighterStatuses& statuses, const formats::SkillTable& skills, int skillId, int special = 0);

    /// @brief Puts a status on (skill) or takes it off (skill 0) by its kind 1..8.
    /// @details Original: RevFightHumStatus, packet 53/1.
    /// @param statuses Fighter's statuses.
    /// @param skills Skill table.
    /// @param kind Status kind 1..8.
    /// @param skillId Skill to put on, 0 to take off.
    /// @param subIndex The lesser status's place for kind 5.
    void setStatus(
        FighterStatuses& statuses,
        const formats::SkillTable& skills,
        int kind,
        int skillId,
        int subIndex
    );

    /// @brief Takes off the status a curing skill cures.
    /// @details Original: ApplyEffectToFighter for curing skills (types 5 and 16).
    /// @param statuses Fighter's statuses.
    /// @param skills Skill table.
    /// @param skillId Curing skill.
    void cureStatus(FighterStatuses& statuses, const formats::SkillTable& skills, int skillId);

    /// @brief Applies one value of an action as a status put on by the action's skill; effect 58 heals as it goes on.
    /// @details Original: ApplyEffectToFighter for one value.
    /// @param statuses Fighter's statuses.
    /// @param skills Skill table.
    /// @param attribute Attribute of the value.
    /// @param skillId The action's skill.
    /// @param total The action's first value (what a lesser status keeps).
    /// @return Whether the value was a status and what became of it.
    StatusValueResult applyStatusValue(
        FighterStatuses& statuses,
        const formats::SkillTable& skills,
        int attribute,
        int skillId,
        std::int32_t total
    );

    /// @brief Gives the one a skill reaches its colour and size.
    /// @details Original: ProcessSkillHit.
    /// @param statuses Fighter's statuses.
    /// @param skills Skill table.
    /// @param skillId Skill that hits.
    /// @param landed Whether the hit landed.
    /// @param missed Whether the hit missed.
    void markStatusHit(
        FighterStatuses& statuses,
        const formats::SkillTable& skills,
        int skillId,
        bool landed,
        bool missed
    );

}

#endif
