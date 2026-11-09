#include "client/subsystems/battle/Battle.h"

#include "client/rules/Skills.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kNoTarget = 0;
        constexpr int kDefence = 60021; // skill 60021: Defense
        constexpr int kFlee = 60041; // skill 60041: Flee

        /// @brief Whether a skill is one of the role stunts that are told as the role's first skill (15003).
        /// @param skillId Skill id.
        /// @return True for a stunt.
        bool isStunt(int skillId) {
            switch (skillId) {
                // role stunt skill ids (told as the role's first skill, 15003)
                case 11075: // Summon Dogs Groups
                case 11076: // Combo x3 Attack
                case 11077: // Cure 2 Players
                case 11078: // Bomber Attack
                case 11182: // Ghost Hammer
                case 11183: // Deacon Attack
                case 12036: // Leap
                case 12049: // Fire Dance
                case 12051: // Note
                case 12053: // Gallop
                case 15038: // Overarm Stumble
                case 15039: // Wine Flame
                case 15040: // Palm
                case 15041: // Love Wish
                case 15060: return true; // Throw Dish
                default: // any other skill is not a stunt
                    return false;
            }
        }

    }

    int Battle::ownFighterSlot() const {
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            if (const auto fighter = m_field.fighter(slot); fighter && fighter->get().kind == 2 && fighter->get().id == m_ownId) {
                return slot;
            }
        }
        return kNoTarget;
    }

    int Battle::ownPetSlot() const {
        const int petId = m_hooks.activePetId ? m_hooks.activePetId() : 0;
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            if (const auto fighter = m_field.fighter(slot);
                fighter && fighter->get().kind == 4 && fighter->get().id == petId && fighter->get().ownerId == m_ownId) {
                return slot;
            }
        }
        return kNoTarget;
    }

    int Battle::findAutoTarget(
        int side,
        int nth,
        int mode,
        int skillId,
        bool isItem
    ) const {
        const std::array<int, 2> rows = side == 2 ? std::array<int, 2>{1, 2} : std::array<int, 2>{4, 3};
        int result = kNoTarget;
        int found = 0;
        int leastHp = 65535; // larger than any HP: start value of the minimum search
        int leastSp = 65535; // larger than any SP: start value of the minimum search
        for (const int row : rows) {
            for (int col = 1; col <= 4; ++col) { // 4 columns per row
                const int slot = m_field.slotAt(row, col);
                const auto fighter = m_field.fighter(slot);
                if (slot == 0 || !fighter || fighter->get().id == 0) {
                    continue;
                }
                const Fighter& candidate = *fighter;
                const auto holds = [&] {
                    return !isItem && std::find(candidate.statuses.icons.begin(), candidate.statuses.icons.end(), skillId) != candidate.statuses.icons.end();
                };
                switch (mode) {
                    case 0: // mode 0: any living fighter
                        if (candidate.hp == 0) {
                            continue;
                        }
                        break;
                    case 1: // mode 1: living fighter with the least HP that is hurt and lacks the skill's status
                        if (candidate.hp == 0 || candidate.hp >= leastHp || candidate.hp >= candidate.maxHp || holds()) {
                            continue;
                        }
                        leastHp = candidate.hp;
                        break;
                    case 2: // mode 2: living fighter with the least SP that lacks SP and the skill's status
                        // 15075: skill 15075 Magic Recovery, which may not be cast on oneself
                        if (candidate.hp <= 0 || candidate.sp >= leastSp || candidate.sp >= candidate.maxSp || holds() ||
                            (!isItem && skillId == 15075 && slot == m_actor)) {
                            continue;
                        }
                        leastSp = candidate.sp;
                        break;
                    case 3: // mode 3: fallen fighter
                        if (candidate.hp != 0) {
                            continue;
                        }
                        break;
                    default: // unknown mode: no target
                        return kNoTarget;
                }
                result = slot;
                if (++found >= nth) {
                    return result;
                }
            }
        }
        return result;
    }

    bool Battle::skillUsableOn(int skillId, int casterSlot, int targetSlot) const {
        if (skillId == 0) {
            return false;
        }
        const auto caster = m_field.fighter(casterSlot);
        const auto target = m_field.fighter(targetSlot);
        const auto skill = m_services.skills.find(skillId);
        if (!caster || !skill) {
            return false;
        }
        if (skillSealed(*caster, skillId)) {
            return false;
        }
        if (skill->targetType == 3 && casterSlot != targetSlot) { // target type 3: oneself only
            return false;
        }
        if (skill->targetType == 5 && casterSlot == targetSlot) { // target type 5: anyone but oneself
            return false;
        }
        return !target || std::find(target->get().statuses.icons.begin(), target->get().statuses.icons.end(), skillId) == target->get().statuses.icons.end();
    }

    bool Battle::chooseItemById(int itemId) {
        const int bagSlot = m_hooks.bagSlotOf ? m_hooks.bagSlotOf(itemId) : 0;
        if (bagSlot == 0 || !m_services.items.find(itemId)) {
            return false;
        }
        m_item = itemId;
        m_itemSlot = bagSlot;
        return true;
    }

    bool Battle::runAutoSettings(const AutoBattleSettings& sets) {
        const int ownSlot = ownFighterSlot();
        const auto own = m_field.fighter(ownSlot);
        const auto actor = m_field.fighter(m_actor);
        if (!own || !actor) {
            return false;
        }
        const int enemySide = own->get().side() == 1 ? 2 : 1;
        const int allySide = actor->get().side() == 1 ? 1 : 2;
        int targetSlot = kNoTarget;
        int actionId = 0;
        bool isItem = false;
        bool done = false;
        const auto targetTypeOf = [&](int skillId) {
            const auto skill = m_services.skills.find(skillId);
            return skill ? skill->targetType : 0;
        };
        const auto chooseSkillStep = [&] {
            if (actionId == kDefence || actionId == kFlee) {
                targetSlot = m_actor;
            }
            if (!isItem && !skillUsableOn(actionId, m_actor, targetSlot)) {
                targetSlot = kNoTarget;
                actionId = 0;
            }
            if (isStunt(actionId)) {
                actionId = kRoleSkillPlaceholder;
            }
            if (actionId != 0) {
                chooseSkill(actionId, false);
            }
            return m_skill != 0;
        };
        const auto heal = [&](int index, int mode) {
            isItem = sets.healIsItem[static_cast<std::size_t>(index)] == 1;
            actionId = sets.healActionIds[static_cast<std::size_t>(index)];
            targetSlot = kNoTarget;
            switch (sets.healTargets[static_cast<std::size_t>(index)]) {
                case 4: // heal target 4: the ally who needs it most
                    // nth = 8: the last one found among the 8 allies
                    targetSlot = findAutoTarget(allySide, 8, mode, actionId, isItem);
                    break;
                case 2: // heal target 2: the player
                    targetSlot = ownSlot;
                    break;
                case 3: // heal target 3: the pet
                    targetSlot = ownPetSlot();
                    break;
                default: // no heal target set
                    break;
            }
            if (const auto target = m_field.fighter(targetSlot); targetSlot != kNoTarget && target) {
                const Fighter& fighter = *target;
                // healThresholds[1] is the HP percent limit, [2] the SP limit
                const bool skip = mode == 1 ? (fighter.hp >= fighter.maxHp || fighter.hp == 0 || fighter.hp >= sets.healThresholds[1])
                                            : (fighter.hp <= 0 || fighter.sp >= fighter.maxSp || fighter.sp >= sets.healThresholds[2]);
                if (skip) {
                    targetSlot = kNoTarget;
                }
            } else {
                targetSlot = kNoTarget;
            }
            if (targetSlot == kNoTarget) {
                return false;
            }
            if (!isItem) {
                const int type = targetTypeOf(actionId);
                if (type == 0) { // target type 0: an enemy
                    targetSlot = findAutoTarget(enemySide, 1, 0, actionId, false);
                } else if (type == 3) { // target type 3: oneself
                    targetSlot = m_actor;
                }
                if (!skillUsableOn(actionId, m_actor, targetSlot)) {
                    actionId = 0;
                    targetSlot = kNoTarget;
                }
                if (isStunt(actionId)) {
                    actionId = kRoleSkillPlaceholder;
                }
                if (actionId != 0) {
                    chooseSkill(actionId, false);
                }
                return m_skill != 0;
            }
            return chooseItemById(actionId);
        };
        const auto steps = [&] {
            // flags[6] / index 3: the heal of the party's death
            if (sets.flags[6] != 0 && sets.healActionIds[3] != 0 && sets.healIsItem[3] != 0) {
                actionId = 0;
                targetSlot = kNoTarget;
                isItem = sets.healIsItem[3] == 1;
                actionId = sets.healActionIds[3];
                targetSlot = findAutoTarget(own->get().side() == 1 ? 1 : 2, 1, 3, actionId, isItem); // mode 3: a fallen ally
                if (targetSlot != kNoTarget) {
                    if (!isItem) {
                        if (targetTypeOf(actionId) == 0) {
                            targetSlot = findAutoTarget(enemySide, 1, 0, actionId, false);
                        }
                        if (chooseSkillStep()) {
                            done = true;
                            return;
                        }
                    } else if (chooseItemById(actionId)) {
                        done = true;
                        return;
                    }
                }
            }
            // flags[5]: the faint rule
            if (!done && sets.flags[5] != 0 && sets.faintActionId != 0) {
                actionId = sets.faintActionId;
                targetSlot = findAutoTarget(enemySide, 1, 0, actionId, false);
                bool condition = false;
                if (const auto target = m_field.fighter(targetSlot); targetSlot != kNoTarget && target) {
                    // faintCompareMode 1: target HP below the value, 2: above it
                    condition = (sets.faintCompareMode == 1 && target->get().hp < sets.faintHpValue) ||
                                (sets.faintCompareMode == 2 && target->get().hp > sets.faintHpValue);
                }
                if (targetSlot != kNoTarget && condition) {
                    if (sets.faintTarget == 2) { // faint target 2: the player
                        targetSlot = ownSlot;
                    } else if (sets.faintTarget == 3) { // faint target 3: the pet
                        targetSlot = ownPetSlot();
                    }
                    if (chooseSkillStep()) {
                        done = true;
                        return;
                    }
                }
            }
            // flags[3] / index 1: the HP heal (mode 1)
            if (!done && sets.flags[3] != 0 && sets.healActionIds[1] != 0 && sets.healIsItem[1] != 0 && heal(1, 1)) {
                done = true;
                return;
            }
            // flags[4] / index 2: the SP heal (mode 2)
            if (!done && sets.flags[4] != 0 && sets.healActionIds[2] != 0 && sets.healIsItem[2] != 0 && heal(2, 2)) {
                done = true;
                return;
            }
            // flags[2]: the degree, three skills in rotation
            if (!done && sets.flags[2] != 0) {
                actionId = 0;
                targetSlot = kNoTarget;
                const std::size_t key = m_actor == 1 ? 1 : 2; // rotation key: 1 the player (actor slot 1), 2 the pet
                int position = m_autoRotation[key];
                for (int pass = 1; pass <= 3; ++pass) { // the degree has three skills
                    ++position;
                    if (position > 3) { // past the third skill
                        if (m_autoRotation[key] != position) {
                            m_autoRotation[key] = position;
                        }
                        break;
                    }
                    const auto index = static_cast<std::size_t>(position);
                    if (sets.procSkillIds[index] == 0 || sets.procSkillTargets[index] == 0) {
                        m_autoRotation[key] = position;
                        break;
                    }
                    targetSlot = kNoTarget;
                    switch (sets.procSkillTargets[index]) {
                        case 1: // skill target 1: an enemy
                            targetSlot = findAutoTarget(enemySide, 1, 0, sets.procSkillTargets[index], false);
                            break;
                        case 2: // skill target 2: the player
                            targetSlot = ownSlot;
                            break;
                        case 3: // skill target 3: the pet
                            targetSlot = ownPetSlot();
                            break;
                        default: // no target set
                            break;
                    }
                    if (const auto target = m_field.fighter(targetSlot); targetSlot != kNoTarget && (!target || target->get().hp <= 0)) {
                        targetSlot = kNoTarget;
                    }
                    m_autoRotation[key] = position;
                    actionId = sets.procSkillIds[index];
                    isItem = sets.procSkillIsItem[index] == 1;
                    if (targetSlot == kNoTarget) {
                        continue;
                    }
                    if (isItem) {
                        if (!chooseItemById(actionId)) {
                            continue;
                        }
                        done = true;
                        break;
                    }
                    if (!chooseSkillStep()) {
                        continue;
                    }
                    done = true;
                    break;
                }
            }
            // flags[1]: the two default skills
            if (!done && sets.flags[1] != 0) {
                for (std::size_t i = 1; i <= 2; ++i) { // two default skills
                    if (sets.defaultSkillIds[i] == 0) {
                        continue;
                    }
                    isItem = false;
                    actionId = sets.defaultSkillIds[i];
                    targetSlot = findAutoTarget(enemySide, 1, 0, actionId, false);
                    if (targetSlot == kNoTarget) {
                        continue;
                    }
                    if (!chooseSkillStep()) {
                        continue;
                    }
                    done = true;
                    break;
                }
            }
            if (done) {
                return;
            }
            actionId = 10001; // Hand Attack skill id
            targetSlot = findAutoTarget(enemySide, 1, 0, actionId, false);
            chooseSkill(actionId, false);
            done = true;
        };
        steps();
        if (targetSlot == kNoTarget || actionId == 0) {
            return false;
        }
        if (!isItem) {
            if (m_skill == kRoleSkillPlaceholder) {
                if (const auto user = actorAsUser()) {
                    m_skill = roleSkillId(user->bodyType, user->head);
                }
            }
            if (m_skill == 0) {
                return false;
            }
        }
        submit(targetSlot);
        return true;
    }

}
