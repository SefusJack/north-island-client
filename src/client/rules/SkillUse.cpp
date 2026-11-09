#include "client/rules/SkillUse.h"

#include "client/rules/Skills.h"
#include "client/data/StarterRoles.h"

namespace client {

    namespace {

        // result codes of checkCanUseSkill
        constexpr int kMayUse = 0; // the skill may be used
        constexpr int kWrongWeapon = 1; // the weapon does not suit the skill
        constexpr int kNoSp = 2; // not enough SP
        constexpr int kNoRecord = 252; // no skill record in the skill table
        constexpr int kNoMotion = 253; // no motion record for the skill
        constexpr int kNoSkill = 254; // no skill chosen (id 0)
        constexpr int kMayNot = 255; // not learnt or not its own
        constexpr int kWeaponSlot = 3; // equip slot 3: the weapon slot
        constexpr int kProfessional = 9; // skill category 9: professional skills

        /// @brief The weapon kind of an item.
        /// @details Original: GetItemWeaponKind; two items count as their own kind.
        /// @param itemId The item id.
        /// @param itemType The item's type.
        /// @return The weapon kind.
        int weaponKindOf(int itemId, int itemType) {
            constexpr int kOwnKind = 250; // made-up weapon kind (real item types are 0..11) for the two bazookas
            return itemId == 20037 || itemId == 20038 ? kOwnKind : itemType; // item ids 20037 Bazooka and 20038 Great Bazooka are weapons of their own kind
        }

        /// @brief Whether the motion allows the weapon's family.
        /// @details Original: IsSkillWeaponUsable; the motion's flag for the weapon's family.
        /// @param itemType The weapon item's type.
        /// @param motion The skill's motion record.
        /// @return True when the weapon suits the skill.
        bool weaponSuits(int itemType, const formats::MotionRecord& motion) {
            std::size_t flag = 0;
            switch (itemType) {
                case 1: // item type 1: weapon family 1
                case 2: // item type 2: weapon family 1
                case 3: // item type 3: weapon family 1
                case 6: // item type 6: weapon family 1
                case 8: // item type 8: weapon family 1
                    flag = 1; // motion flag index of weapon family 1
                    break;
                case 0: // item type 0 (no weapon): weapon family 2
                case 7: // item type 7: weapon family 2
                case 9: // item type 9: weapon family 2
                    flag = 2; // motion flag index of weapon family 2
                    break;
                case 5: // item type 5: weapon family 3
                case 10: // item type 10: weapon family 3
                    flag = 3; // motion flag index of weapon family 3
                    break;
                case 4: // item type 4: weapon family 4
                    flag = 4; // motion flag index of weapon family 4
                    break;
                case 11: // item type 11: weapon family 5
                    flag = 5; // motion flag index of weapon family 5
                    break;
                default: // any other type: always suits
                    return true;
            }
            return motion.usableWeapons[flag] == 1;
        }

        /// @brief Whether the fighter has the SP a skill costs.
        /// @details Original: HasEnoughSpForSkill; a cost of 1 asks for any SP at all.
        /// @param skills The skill table.
        /// @param sp The fighter's SP.
        /// @param skillId The skill id.
        /// @return True when enough.
        bool enoughSp(const formats::SkillTable& skills, int sp, int skillId) {
            const auto skill = skills.find(skillId);
            if (!skill) {
                return false;
            }
            return skill->spCost == 1 ? sp > 0 : skill->spCost <= sp;
        }

    }

    int weaponAttackSkill(int weaponKind, int npcVariant) {
        switch (weaponKind) {
            case 0: // weapon kind 0 (bare hands)
                return 10001; // skill 10001: Hand Attack
            case 1: // weapon kind 1: knives and falchions
            case 2: // weapon kind 2: swords
            case 6: // weapon kind 6: claws
            case 8: // weapon kind 8: axes
                return 10002; // skill 10002: Blade Attack
            case 7: // weapon kind 7 (no items in the data)
            case 9: // weapon kind 9: clubs and pestles
                return 10003; // skill 10003: Pestle Attack
            case 5: // weapon kind 5: wands and staves
            case 10: // weapon kind 10: fans
                return 10004; // skill 10004: Spell Attack
            case 3: // weapon kind 3: spears and halberds
                return 10005; // skill 10005: Sword Attack
            case 4: // weapon kind 4 (bows): attack differs for a player and a pet
                return npcVariant == 0 ? 10019 : (npcVariant == 1 ? 10020 : 0); // skill ids: player 10019 Bow Attack, pet 10020 NPC Bow Attack
            case 11: // weapon kind 11 (guns): attack differs for a player and a pet
                return npcVariant == 0 ? 10007 : (npcVariant == 1 ? 10018 : 0); // skill ids: player 10007 Gun Attack, pet 10018 NPC Gun Attack
            case 250: // weapon kind 250: the two items of their own kind (see weaponKindOf)
                return npcVariant == 0 ? 10026 : (npcVariant == 1 ? 10027 : 0); // skill ids: player 10026 Bazooka Attack, pet 10027 NPC Bazooka Attack
            default: // any other weapon kind: no attack
                return 0;
        }
    }

    bool isBattleListSkill(const formats::SkillTable& skills, int skillId) {
        const auto skill = skills.find(skillId);
        return skill && skill->category >= 1 && skill->category <= 7 && skill->key != 0; // skill categories 1..7 are listed in battle
    }

    int checkCanUseSkill(const formats::SkillTable& skills, const formats::MotionTable& motions, const SkillUser& user, int& skillId) {
        const auto refuse = [&](int result) {
            skillId = 0;
            return result;
        };
        if (skillId == 0) {
            return refuse(kNoSkill);
        }
        const auto motion = motions.find(skillId);
        if (!motion) {
            return refuse(kNoMotion);
        }
        const auto skill = skills.find(skillId);
        if (!skill) {
            return refuse(kNoRecord);
        }
        if (skill->skillType >= 10 && skill->skillType <= 12) { // skill types 10..12: defend, capture, flee
            return kMayUse;
        }
        if (user.weaponId != 0 && (!user.weapon || user.weapon->equipSlot != kWeaponSlot)) {
            return refuse(kMayNot);
        }
        const bool ranked = user.weapon && user.weapon->rank != 0;
        const int weaponType = ranked ? user.weapon->type : 0;
        const int weaponKind = ranked ? weaponKindOf(user.weapon->id, user.weapon->type) : 0;
        const bool learnt = skill->category != 0;
        switch (user.kind) {
            case 1: // fighter kind 1: a player
            case 2: // fighter kind 2: a player
            case 9: // fighter kind 9: a player
                if (!learnt) {
                    skillId = weaponAttackSkill(weaponKind, 0);
                } else {
                    if (!user.skillLevel || user.skillLevel(skill->key) <= 0) {
                        return refuse(kMayNot);
                    }
                    if (skillId == kRoleSkillPlaceholder) {
                        if (starterEquipment(roleIndex(user.bodyType, user.head))[1] != user.clothesId) { // index 1: the clothes slot of the starter equipment
                            return refuse(kMayNot);
                        }
                        const auto roleMotion = motions.find(roleSkillId(user.bodyType, user.head));
                        if (!roleMotion || !weaponSuits(weaponType, *roleMotion)) {
                            return refuse(kWrongWeapon);
                        }
                    } else if (!weaponSuits(weaponType, *motion)) {
                        return refuse(kWrongWeapon);
                    }
                }
                break;
            case 4: { // fighter kind 4: a pet
                if (!user.npc) {
                    return refuse(kMayNot);
                }
                const formats::NpcInfo& npc = *user.npc;
                const auto ownSkill = [&]() -> std::optional<std::size_t> {
                    for (std::size_t j = 0; j < npc.skills.size(); ++j) {
                        if (npc.skills[j] == skillId && user.petSkillLevels[j] > 0) {
                            return j;
                        }
                    }
                    return std::nullopt;
                };
                switch (npc.directionKind) {
                    case 0: // pet direction kind 0: attacks with its fixed skill or the weapon's attack
                    case 2: // pet direction kind 2: same as kind 0
                        if (!learnt) {
                            skillId = npc.fixedSkill != 0 ? npc.fixedSkill : weaponAttackSkill(weaponKind, 1);
                        } else {
                            if (!ownSkill()) {
                                return refuse(kMayNot);
                            }
                            if (!weaponSuits(weaponType, *motion)) {
                                return refuse(kWrongWeapon);
                            }
                        }
                        break;
                    case 1: // pet direction kind 1: attack chosen by the pet's weapon
                        if (!learnt) {
                            if (npc.fixedSkill != 0) {
                                skillId = npc.fixedSkill;
                            } else if (npc.weapon == 0) {
                                skillId = 10023; // skill 10023: Monster Attack, for pet weapon 0
                            } else if (npc.weapon == 1 || npc.weapon == 2) {
                                skillId = 10010; // skill 10010: Monster Attack, for pet weapons 1 and 2
                            }
                        } else if (!ownSkill()) {
                            return refuse(kMayNot);
                        }
                        break;
                    default: // any other pet direction kind: no check
                        break;
                }
                break;
            }
            default: // any other fighter kind: no check
                break;
        }
        const int costOf = skillId == kRoleSkillPlaceholder ? roleSkillId(user.bodyType, user.head) : skillId;
        if (!enoughSp(skills, user.sp, costOf)) {
            return refuse(kNoSp);
        }
        return kMayUse;
    }

    std::optional<std::pair<std::string, int>> skillUseMessage(const formats::SkillTable& skills, int result, int skillId) {
        constexpr int kShowMs = 1500; // how long the message shows, ms
        switch (result) {
            case kWrongWeapon: // result 1: the weapon does not suit the skill
                return std::make_pair(std::string("A used weapon is not suitable for this skill"), kShowMs);
            case kNoSp: // result 2: not enough SP
                return std::make_pair(std::string("Unable to use without enough SP"), kShowMs);
            case kNoRecord: // result 252: no skill record
                return std::make_pair(std::string("EXCEL") + " table has no data for this skill", kShowMs);
            case kNoMotion: { // result 253: no motion for the skill
                constexpr int kProfessionalMs = 1200; // how long the professional-skill message shows, ms
                const auto skill = skills.find(skillId);
                if (skill && skill->category == kProfessional) {
                    return std::make_pair(std::string("It is a professional skill."), kProfessionalMs);
                }
                return std::make_pair(std::string("Unable to use without proper skill"), kShowMs);
            }
            case kMayNot: // result 255: not learnt or not its own
                return std::make_pair(std::string("Unable to use this skill"), kShowMs);
            default: // result 0 or any other: no message
                return std::nullopt;
        }
    }

}
