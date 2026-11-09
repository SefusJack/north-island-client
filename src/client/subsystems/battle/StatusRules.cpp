#include "client/subsystems/battle/StatusRules.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kShrunk = 172; // status effect 172: shrunk figure (main place 3)
        constexpr int kEnlarged = 55; // status effect 55: enlarged figure (lesser place 7)
        constexpr int kStatus53 = 53; // status effect 53: replaces the figure with a sign sprite (lesser place 7)
        constexpr int kHealing = 58; // status effect 58: the healing status (lesser place 6)
        constexpr int kCureAll = 80; // status effect 80: cures every status that is not a learnt skill's
        constexpr int kSizeBig = 1; // size mode 1: figure drawn bigger
        constexpr int kSizeSmall = 2; // size mode 2: figure drawn smaller

        /// @brief The status effect of a skill.
        /// @param skills Skill table.
        /// @param skillId Skill asked about.
        /// @return Its status effect, 0 if the skill is unknown.
        int effectOf(const formats::SkillTable& skills, int skillId) {
            const auto skill = skills.find(skillId);
            return skill ? skill->statusEffect : 0;
        }

        /// @brief Whether the skill replaces what was in its place.
        /// @param skillId Skill asked about.
        /// @return True for skills 17013 and 17018.
        bool replaces(int skillId) { return skillId == 17013 || skillId == 17018; } // skill 17013: Gephil's Mutter, skill 17018: Gaea's Guard

        /// @brief Empties a main status place and removes its icon.
        /// @param statuses Fighter's statuses.
        /// @param skills Skill table.
        /// @param kind Main status place.
        void clearMain(FighterStatuses& statuses, const formats::SkillTable& skills, int kind) {
            StatusSlot& slot = statuses.main[static_cast<std::size_t>(kind)];
            removeStatusIcon(statuses, skills, slot.skill);
            slot = {};
        }

        /// @brief Empties a lesser status place, its value and its icon.
        /// @param statuses Fighter's statuses.
        /// @param skills Skill table.
        /// @param index Lesser status place.
        void clearSub(FighterStatuses& statuses, const formats::SkillTable& skills, int index) {
            StatusSlot& slot = statuses.sub[static_cast<std::size_t>(index)];
            removeStatusIcon(statuses, skills, slot.skill);
            statuses.values[static_cast<std::size_t>(index)] = 0;
            slot = {};
        }

        /// @brief Lets the skill colour the figure if no skill does yet.
        /// @details Original: the first status giving a colour colours the figure (53/1 only).
        /// @param statuses Fighter's statuses.
        /// @param skills Skill table.
        /// @param skillId Skill that may give the colour.
        void tintFrom(FighterStatuses& statuses, const formats::SkillTable& skills, int skillId) {
            const auto skill = skills.find(skillId);
            if (statuses.tintSkill == 0 && skill && skill->tintKind != 0) {
                statuses.tintKind = skill->tintKind;
                statuses.tintSkill = skillId;
            }
        }

        /// @brief Puts a skill's status into a main place with its icon.
        /// @param statuses Fighter's statuses.
        /// @param skills Skill table.
        /// @param kind Main status place.
        /// @param skillId Skill put on.
        void putMain(FighterStatuses& statuses, const formats::SkillTable& skills, int kind, int skillId) {
            StatusSlot& slot = statuses.main[static_cast<std::size_t>(kind)];
            if (replaces(skillId)) {
                removeStatusIcon(statuses, skills, slot.skill);
            }
            slot = {effectOf(skills, skillId), skillId, false};
            addStatusIcon(statuses, skillId);
        }

        /// @brief Puts a skill's status into a lesser place with its value and icon.
        /// @param statuses Fighter's statuses.
        /// @param skills Skill table.
        /// @param index Lesser status place.
        /// @param skillId Skill put on.
        /// @param value Value kept for the status.
        void putSub(
            FighterStatuses& statuses,
            const formats::SkillTable& skills,
            int index,
            int skillId,
            std::int32_t value
        ) {
            StatusSlot& slot = statuses.sub[static_cast<std::size_t>(index)];
            if (replaces(skillId)) {
                removeStatusIcon(statuses, skills, slot.skill);
            }
            statuses.values[static_cast<std::size_t>(index)] = value;
            slot = {effectOf(skills, skillId), skillId, false};
            addStatusIcon(statuses, skillId);
        }

    }

    void addStatusIcon(FighterStatuses& statuses, int skillId, int special) {
        if (skillId == 0) {
            return;
        }
        if (special == 1 || special == 2) {
            statuses.special[static_cast<std::size_t>(special)] = skillId;
            return;
        }
        if (special != 0 || std::find(statuses.icons.begin(), statuses.icons.end(), skillId) != statuses.icons.end()) {
            return;
        }
        if (static_cast<int>(statuses.icons.size()) < FighterStatuses::kMostIcons) {
            statuses.icons.push_back(skillId);
        }
    }

    void removeStatusIcon(FighterStatuses& statuses, const formats::SkillTable& skills, int skillId, int special) {
        if (skillId == 0) {
            return;
        }
        if (skillId == statuses.tintSkill) {
            statuses.tintKind = 0;
            statuses.tintSkill = 0;
        }
        if (special == 1 || special == 2) {
            statuses.special[static_cast<std::size_t>(special)] = 0;
            return;
        }
        std::vector<int> kept;
        for (const int icon : statuses.icons) {
            if (icon == 0 || icon == skillId) {
                continue;
            }
            tintFrom(statuses, skills, icon);
            kept.push_back(icon);
        }
        statuses.icons = std::move(kept);
    }

    void setStatus(
        FighterStatuses& statuses,
        const formats::SkillTable& skills,
        int kind,
        int skillId,
        int subIndex
    ) {
        // status kinds 1..8; lesser status places 1..8
        if (kind < 1 || kind > 8) {
            return;
        }
        const bool sub = subIndex >= 1 && subIndex <= 8;
        if (skillId == 0) {
            switch (kind) {
                case 3: // kind 3 taken off: main place 3
                case 8: // kind 8 taken off: main place 8
                    if (statuses.main[static_cast<std::size_t>(kind)].effect == kShrunk) {
                        statuses.sizeMode = 0;
                    }
                    clearMain(statuses, skills, kind);
                    break;
                case 5: // kind 5 taken off: the lesser status at subIndex
                    if (!sub) {
                        break;
                    }
                    if (statuses.sub[static_cast<std::size_t>(subIndex)].effect == kEnlarged) {
                        statuses.sizeMode = 0;
                    }
                    if (statuses.sub[static_cast<std::size_t>(subIndex)].effect == kStatus53) {
                        statuses.sub53 = false;
                    }
                    clearSub(statuses, skills, subIndex);
                    break;
                case 6: // kind 6 taken off: special place 1 (kind - 5)
                case 7: // kind 7 taken off: special place 2 (kind - 5)
                    removeStatusIcon(statuses, skills, statuses.special[static_cast<std::size_t>(kind - 5)], kind - 5);
                    break;
                default: // kinds 1, 2, 4: the main place
                    clearMain(statuses, skills, kind);
                    break;
            }
            return;
        }
        const int effect = effectOf(skills, skillId);
        // lesser place 7 holds the size-changing statuses (enlarged / 53); main place 3 holds the shrinking one
        switch (kind) {
            case 3: // kind 3 put on: main place 3
                if (effect == kShrunk && statuses.sub[7].effect == kEnlarged) {
                    clearSub(statuses, skills, 7);
                    statuses.sizeMode = 0;
                    break;
                }
                if (effect == kShrunk) {
                    statuses.sizeMode = kSizeSmall;
                }
                putMain(statuses, skills, kind, skillId);
                tintFrom(statuses, skills, skillId);
                break;
            case 5: // kind 5 put on: the lesser status at subIndex
                if (!sub) {
                    break;
                }
                if (effect == kEnlarged && statuses.main[3].effect == kShrunk) {
                    statuses.sizeMode = 0;
                    clearMain(statuses, skills, 3);
                    break;
                }
                if (effect == kEnlarged) {
                    statuses.sizeMode = kSizeBig;
                }
                putSub(statuses, skills, subIndex, skillId, 0);
                if (effect == kStatus53) {
                    statuses.sub53 = true;
                }
                tintFrom(statuses, skills, skillId);
                break;
            case 6: // kind 6 put on: special place 1 (kind - 5)
            case 7: // kind 7 put on: special place 2 (kind - 5)
                addStatusIcon(statuses, skillId, kind - 5);
                break;
            default: // kinds 1, 2, 4, 8: the main place
                putMain(statuses, skills, kind, skillId);
                tintFrom(statuses, skills, skillId);
                break;
        }
    }

    void cureStatus(FighterStatuses& statuses, const formats::SkillTable& skills, int skillId) {
        const int effect = effectOf(skills, skillId);
        const auto learntSkill = [&](int kind) {
            const auto skill = skills.find(statuses.main[static_cast<std::size_t>(kind)].skill);
            return skill && skill->category == 1; // skill category 1: a learnt skill
        };
        // status effect ranges: 1..50 main place 1, 51..100 lesser places, 101..170 main place 2, 171..240 main place 3
        if (effect >= 1 && effect <= 50) {
            if (statuses.main[1].effect != 0 && statuses.main[1].effect == effect) {
                clearMain(statuses, skills, 1);
            }
        } else if (effect >= 51 && effect <= 100) {
            switch (effect) {
                case kCureAll: // effect 80: cures every status of main places 1..3 that is not a learnt skill's
                    for (const int kind : {1, 2, 3}) {
                        if (statuses.main[static_cast<std::size_t>(kind)].effect != 0 && !learntSkill(kind)) {
                            clearMain(statuses, skills, kind);
                        }
                    }
                    break;
                case 51: // effect 51: cures lesser place 5
                    clearSub(statuses, skills, 5);
                    break;
                case 52: // effect 52: cures lesser place 2
                    clearSub(statuses, skills, 2);
                    break;
                case 59: // effect 59: cures lesser place 1
                    clearSub(statuses, skills, 1);
                    break;
                case kStatus53: // effect 53: cures lesser place 7
                case kEnlarged: // effect 55: cures lesser place 7 (enlarged)
                    if (statuses.sub[7].effect == kEnlarged) {
                        statuses.sizeMode = 0;
                    }
                    if (statuses.sub[7].effect == kStatus53) {
                        statuses.sub53 = false;
                    }
                    clearSub(statuses, skills, 7);
                    break;
                case kHealing: // effect 58: cures lesser place 6 (the healing status)
                    clearSub(statuses, skills, 6);
                    break;
                case 64: // effect 64: cures lesser place 8
                    clearSub(statuses, skills, 8);
                    break;
                default: // other effects cure nothing
                    break;
            }
        } else if (effect >= 101 && effect <= 170) {
            if (statuses.main[2].effect != 0 && statuses.main[2].effect == effect) {
                clearMain(statuses, skills, 2);
            }
        } else if (effect >= 171 && effect <= 240) {
            if (statuses.main[3].effect != 0 && statuses.main[3].effect == effect) {
                if (statuses.main[3].effect == kShrunk) {
                    statuses.sizeMode = 0;
                }
                clearMain(statuses, skills, 3);
            }
        }
    }

    StatusValueResult applyStatusValue(
        FighterStatuses& statuses,
        const formats::SkillTable& skills,
        int attribute,
        int skillId,
        std::int32_t total
    ) {
        const int effect = effectOf(skills, skillId);
        switch (attribute) {
            case 221: // attribute 221: status in main place 1
                putMain(statuses, skills, 1, skillId);
                break;
            case 222: // attribute 222: status in main place 2
                putMain(statuses, skills, 2, skillId);
                break;
            case 223: // attribute 223: status in main place 3 (a shrinking status cancels an enlarged one)
                if (effect == kShrunk && statuses.sub[7].effect == kEnlarged) {
                    clearSub(statuses, skills, 7);
                    statuses.sizeMode = 0;
                    break;
                }
                putMain(statuses, skills, 3, skillId);
                break;
            case 224: // attribute 224: status in main place 4
                putMain(statuses, skills, 4, skillId);
                break;
            case 225: // attribute 225: lesser place 1 (attribute - 224)
            case 226: // attribute 226: lesser place 2
            case 227: // attribute 227: lesser place 3
            case 228: // attribute 228: lesser place 4
            case 229: // attribute 229: lesser place 5
                putSub(statuses, skills, attribute - 224, skillId, total);
                break;
            case 230: // attribute 230: lesser place 7 (enlarged / 53 statuses)
                if (effect == kEnlarged && statuses.main[3].effect == kShrunk) {
                    statuses.sizeMode = 0;
                    clearMain(statuses, skills, 3);
                    break;
                }
                putSub(statuses, skills, 7, skillId, 0);
                if (effect == kStatus53) {
                    statuses.sub53 = true;
                }
                break;
            case 232: // attribute 232: lesser place 6 (keeps its value; the healing effect becomes HP)
                putSub(statuses, skills, 6, skillId, statuses.values[6]);
                return effect == kHealing ? StatusValueResult::BecomesHp : StatusValueResult::Applied;
            case 233: // attribute 233: lesser place 8
                putSub(statuses, skills, 8, skillId, total);
                break;
            case 234: // attribute 234: special icon place 1
                addStatusIcon(statuses, skillId, 1);
                break;
            case 235: // attribute 235: special icon place 2
                addStatusIcon(statuses, skillId, 2);
                break;
            case 236: // attribute 236: status in main place 8
                putMain(statuses, skills, 8, skillId);
                break;
            default: // any other attribute is not a status
                return StatusValueResult::NotStatus;
        }
        return StatusValueResult::Applied;
    }

    void markStatusHit(
        FighterStatuses& statuses,
        const formats::SkillTable& skills,
        int skillId,
        bool landed,
        bool missed
    ) {
        const auto skill = skills.find(skillId);
        if (!skill) {
            return;
        }
        const int effect = skill->statusEffect;
        if (landed && skill->tintKind != 0) {
            // effect ranges 1..50 (main place 1) and 171..240 (main places 3 and 8)
            const bool coloured = (effect >= 1 && effect <= 50 && statuses.main[1].effect == effect) ||
                                  (effect >= 171 && effect <= 240 && (statuses.main[3].effect == effect || statuses.main[8].effect == effect));
            if (coloured) {
                statuses.tintKind = skill->tintKind;
                statuses.tintSkill = skillId;
            }
        }
        if (!missed) {
            if (effect == kShrunk) {
                statuses.sizeMode = statuses.sub[7].effect != kEnlarged ? kSizeSmall : 0;
            } else if (effect == kEnlarged) {
                statuses.sizeMode = statuses.main[3].effect != kShrunk ? kSizeBig : 0;
            }
        }
        if (skill->skillType == 5 || skill->skillType == 16) { // skill types 5 and 16: curing skills
            if (statuses.tintSkill != 0 && effect == effectOf(skills, statuses.tintSkill)) {
                statuses.tintKind = 0;
                statuses.tintSkill = 0;
            }
            if ((effect == statuses.main[3].effect || effect == kCureAll) && statuses.sizeMode == kSizeSmall) {
                statuses.sizeMode = 0;
            }
        }
    }

}
