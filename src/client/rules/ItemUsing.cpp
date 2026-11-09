#include "client/rules/ItemUsing.h"

#include "client/rules/ItemInfo.h"

namespace client {

    namespace {

        constexpr int kNeededAttribute = 64; // attribute kind 64: pet amity (items with it are for pets only)
        constexpr int kHealKind = 25; // item sub type 25: heal items
        constexpr int kExpKind = 113; // item sub type 113: exp items
        constexpr int kExpBookKind = 114; // item sub type 114: exp book items
        constexpr int kBottleKind = 151; // item sub type 151: HP/SP bottles
        constexpr std::int64_t kMostExp = 656110389; // exp cap of a normal character
        constexpr std::int64_t kMostRebornExp = 1995558935; // exp cap of a reborn character

        // item sub types that cannot be used in combat: 113-114 (exp items), 127-128, 143-144

        /// @brief Whether a sub type is one of those that cannot be used in combat.
        /// @param kind The item sub type.
        /// @return True when in the ranges.
        bool inKinds(int kind) { return (kind >= 113 && kind <= 114) || (kind >= 127 && kind <= 128) || (kind >= 143 && kind <= 144); }

    }

    UseRefusal useRefusal(const formats::ItemRecord& item, const UseTarget& target, bool fighting) {
        constexpr const char* kNotNeeded = "Don't need it now";
        constexpr const char* kUnavailable = "The target is unavailable!";
        constexpr const char* kNotByPlayer = "This item cannot be used by the player";
        const std::int64_t mostExp = target.reborn > 0 ? kMostRebornExp : kMostExp;
        switch (item.subType) {
            case 0: // sub type 0: plain item; attribute 64 is for pets only
                for (const int kind : item.attributeKinds) {
                    if (kind != kNeededAttribute) {
                        continue;
                    }
                    if (!target.pet) {
                        return {kNotByPlayer, 2000}; // message time in ms
                    }
                    if (target.mercenary) {
                        return {kUnavailable, 1500}; // message time in ms
                    }
                }
                break;
            case kHealKind: // sub type 25: heal item, not needed at full HP
                if (target.hp >= target.maxHp) {
                    return {kNotNeeded, 2000}; // message time in ms
                }
                break;
            case kExpKind: // sub type 113: exp item, refused at the max exp
                if (target.exp >= mostExp) {
                    return {"At the max EXP, cannot use", 2000}; // message time in ms
                }
                break;
            case 127: // sub type 127: not for pets
                if (target.pet) {
                    return {"This item cannot be used", 2000}; // message time in ms
                }
                break;
            case 128: // sub type 128: players only
                if (target.pet) {
                    return {"Only for players", 2000}; // message time in ms
                }
                break;
            case kBottleKind: { // sub type 151: bottle; kinds 1/3 refill HP, 2/4 refill SP
                const int bottle = bottleKind(item);
                if ((bottle == 1 || bottle == 3) && target.hp >= target.maxHp) {
                    return {kNotNeeded, 2000}; // message time in ms
                }
                if ((bottle == 2 || bottle == 4) && target.sp >= target.maxSp) {
                    return {kNotNeeded, 2000}; // message time in ms
                }
                break;
            }
            default: // any other sub type: no extra check
                break;
        }
        if (target.pet && target.tradable == 0) {
            return {"The quest NPC is unable to use the item", 1200}; // message time in ms
        }
        if (inKinds(item.subType) && fighting) {
            return {"Unable to use in combat", 1500}; // message time in ms
        }
        if (target.pet && target.mercenary && (item.subType == kExpKind || item.subType == kExpBookKind)) {
            return {kUnavailable, 1500}; // message time in ms
        }
        constexpr int kPetsOnly = 8; // usableBy 8: pets only
        constexpr int kForgetsSkill = 3; // usableBy 3: makes the target forget a skill
        if (item.usableBy == kPetsOnly && !target.pet) {
            return {kNotByPlayer, 1200}; // message time in ms
        }
        if (item.usableBy == kForgetsSkill && target.pet) {
            return {"Will forget the following skill, are you sure? ", 1200}; // message time in ms
        }
        return {};
    }

}
