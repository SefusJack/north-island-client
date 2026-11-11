#include "client/rules/ItemInfo.h"

#include "core/DateTime.h"
#include "ui/TextWrap.h"

#include <array>
#include <cmath>

namespace client {

    namespace {

        constexpr int kQuestItemType = 21; // item type 21: quest props
        constexpr int kNoTradeFlag = 2; // item flag bit 2: cannot be transferred
        constexpr int kNeutralValue = 100; // stored attribute value meaning "no change"; shown value = stored - 100
        constexpr int kHiddenAttribute = 250; // attribute kind 250: hidden, never listed in the popup
        constexpr int kBottleSubType = 151; // item sub type 151: HP/SP bottles
        constexpr int kWornOut = 250; // wear value at which an item is destroyed (wear runs 0..250)

        /// @brief Names an item attribute kind for the info popup.
        /// @param kind The attribute kind.
        /// @return The label, empty for a kind with none.
        std::string attributeName(int kind) {
            switch (kind) {
                case 25: return "HP"; // attribute kind 25: HP
                case 26: return "SP"; // attribute kind 26: SP
                case 207: return "MaxHp"; // attribute kind 207: max HP
                case 208: return "MaxSp"; // attribute kind 208: max SP
                case 210: return "ATK"; // attribute kind 210: attack
                case 211: return "DEF"; // attribute kind 211: defence
                case 215: return "MATK"; // attribute kind 215: magic attack
                case 216: return "MDEF"; // attribute kind 216: magic defence
                case 214: return "SPD"; // attribute kind 214: speed
                case 64: return "Amity Degree"; // attribute kind 64: pet amity
                case 36: return "Exp"; // attribute kind 36: experience
                case 100: return "resistance of shoot"; // attribute kind 100: shoot resistance
                default: return ""; // any other kind: no name
            }
        }

        /// @brief Whether a fusion level raises this attribute kind.
        /// @param kind The attribute kind.
        /// @return True for the stat attributes.
        bool fusionRaises(int kind) {
            // attribute kinds: 25 HP, 26 SP, 207 max HP, 208 max SP, 210 ATK, 211 DEF, 214-216 SPD/MATK/MDEF
            return kind == 25 || kind == 26 || kind == 207 || kind == 208 || kind == 210 || kind == 211 || (kind >= 214 && kind <= 216);
        }

        /// @brief Decodes the high nibble of an attribute adjust byte as a signed value.
        /// @details Original: DecodeSignedNibble (sign in bit 7, value in bits 4..6).
        /// @param value The adjust byte.
        /// @return The signed adjustment.
        int signedNibble(int value) {
            // bit 7 is the sign, bits 4..6 the value; 8 is the sign's weight in the nibble
            const int sign = (value >> 7) & 1;
            return ((value >> 4) - sign * 8) * -(sign * 2 - 1);
        }

        /// @brief Wear state line of the item info popup.
        /// @param wear The item's wear (250 is destroyed).
        /// @return The "Status:" text.
        std::string wearStatus(int wear) {
            if (wear < 150) { // wear below 150: good
                return "Status:  Good";
            }
            if (wear < 200) { // wear below 200: usable
                return "Status:  Usable";
            }
            if (wear < kWornOut) {
                return "Status:  Wearing";
            }
            return "Status:  Destroyed";
        }

        /// @brief Whether an item is a gem.
        /// @param items The item table.
        /// @param itemId The item id.
        /// @return True for item types 52 and 54.
        bool isGem(const formats::ItemTable& items, int itemId) {
            const auto gem = items.find(itemId);
            return gem && (gem->type == 52 || gem->type == 54); // item types 52 and 54: gems
        }

        /// @brief Wear state text of a stall list entry.
        /// @details Original: the status line of RecvStallItemList (IsWearRangeSpecial and IsWear100Special are always false).
        /// @param wear The item's wear.
        /// @return The "Status:" text.
        std::string stallWearStatus(int wear) {
            if (wear < 150) { // wear below 150: good
                return "Status: Good";
            }
            if (wear < 200) { // wear below 200: usable
                return "Status: Usable";
            }
            if (wear < kWornOut) {
                return "Status: Wearing";
            }
            return "Status: Destroyed";
        }

        /// @brief Kind of an item worn in equip slot 9.
        /// @details Original: GetSlot9ItemKind.
        /// @param item The item record.
        /// @return 0 not slot 9, 1 id 34059, 2 id 34194 (box item), else 255.
        int slot9Kind(const formats::ItemRecord& item) {
            if (item.equipSlot != 9) { // equip slot 9
                return 0;
            }
            return item.id == 34059 ? 1 : item.id == 34194 ? 2 : 255; // item ids 34059 (Token Egg) / 34194 (Present Case) map to kinds 1 / 2, others 255
        }

        /// @brief The attribute line of a stall list entry, e.g. "ATK:+5", with the gem's and fusion's bonus after each.
        /// @param items The item table.
        /// @param record The item record.
        /// @param item The item instance.
        /// @return The attribute line.
        std::string stallAttributes(const formats::ItemTable& items, const formats::ItemRecord& record, const protocol::ItemInstance& item) {
            const bool equipment = record.equipSlot >= 1 && record.equipSlot <= 6; // equip slots 1..6: gear
            int fusionBonus = 0;
            if (record.equipSlot >= 1 && record.equipSlot <= 5 && item.fusionLevel > 0) { // equip slots 1..5: worn gear that takes fusion
                int raised = 0;
                for (std::size_t k = 0; k < 2; ++k) { // two attribute slots per item
                    const int kind = record.attributeKinds[k];
                    // attribute kinds 25, 26, 207, 208, 210, 211, 214-216: the stats a fusion raises
                    if ((kind == 25 || kind == 26 || kind == 207 || kind == 208 || kind == 210 || kind == 211 || (kind >= 214 && kind <= 216)) &&
                        record.attributeValues[k] > kNeutralValue) {
                        ++raised;
                    }
                    fusionBonus = raised == 1 ? item.fusionLevel * 2 : raised == 2 ? item.fusionLevel : fusionBonus;
                }
            }
            int gemBonus = 0;
            if (equipment && item.insertItemId != 0 && isGem(items, item.insertItemId)) {
                const auto gem = items.find(item.insertItemId);
                if (gem && gem->type == 52) { // item type 52: gem that adds a stat bonus
                    int strong = 0;
                    for (std::size_t k = 0; k < 2; ++k) { // two attribute slots per item
                        const int kind = record.attributeKinds[k];
                        if (kind != 0 && kind != kHiddenAttribute && record.attributeValues[k] > kNeutralValue) {
                            ++strong;
                        }
                    }
                    gemBonus = strong == 1 ? gem->rank * 2 : strong == 2 ? gem->rank : 0;
                }
            }
            std::array<int, 2> adjust{}; // one adjust per attribute slot
            if (item.attributeAdjust > 0) {
                adjust[0] = signedNibble(item.attributeAdjust);
                adjust[1] = signedNibble((item.attributeAdjust & 0x0F) << 4); // low nibble moved to the high nibble
            }
            std::string line;
            for (std::size_t k = 0; k < 2; ++k) { // two attribute slots per item
                const int kind = record.attributeKinds[k];
                const int value = record.attributeValues[k];
                if (kind == kHiddenAttribute || value == 0 || value == kNeutralValue) {
                    continue;
                }
                const std::string name = kind == 36 ? std::string() : attributeName(kind); // attribute kind 36 (exp) has no name
                if (!name.empty()) {
                    line = k == 1 && !line.empty() ? line + ", " + name : name;
                }
                const int shown = value - kNeutralValue + adjust[k];
                line += shown < 0 ? ":-" + std::to_string(-shown) : ":+" + std::to_string(shown);
                if (kind == 100) { // attribute kind 100: shoot resistance, shown in percent
                    line += "%";
                }
                if (gemBonus > 0 && value > kNeutralValue) {
                    line += "+" + std::to_string(gemBonus);
                }
                if (fusionBonus > 0 && value > kNeutralValue) {
                    line += "+" + std::to_string(fusionBonus);
                }
            }
            return line;
        }

    }

    int usePeriod(const formats::ItemRecord& item) {
        const bool lasting = (item.equipSlot >= 1 && item.equipSlot <= 7) || item.equipSlot == 10; // equip slots 1..7 and 10 wear out with time
        return lasting && item.usePeriod > 0 ? item.usePeriod : 0;
    }

    bool isMallEquipment(const formats::ItemRecord& item) {
        constexpr int kMallFlag = 0x200; // item flag bit 0x200: item-mall equipment
        // shop picture ids above 1000 are mall pictures; equip slots 1..6 are gear
        return item.shopPicId > 1000 && (item.flags & kMallFlag) == kMallFlag && item.equipSlot >= 1 && item.equipSlot <= 6;
    }

    int bottleKind(const formats::ItemRecord& item) {
        if (item.subType != kBottleSubType) {
            return 0;
        }
        if (item.id == 34298) { // item id 34298: HP Bottle (special HP bottle)
            return 3;
        }
        if (item.id == 34299) { // item id 34299: SP Bottle (special SP bottle)
            return 4;
        }
        return item.attributeKinds[0] == 25 ? 1 : item.attributeKinds[0] == 26 ? 2 : 0; // attribute kind 25 (HP) -> bottle 1, 26 (SP) -> bottle 2
    }

    bool isMultiUse(const formats::ItemRecord& item) {
        if (item.maxUses == 0) {
            return false;
        }
        switch (item.id) {
            case 25221: // Coffee Pack
            case 25620: // Super Caffeine Pack
            case 34132: // Loudspeaker
            case 34152: // Tear Plier
            case 39074: // Magic Sandglass
            case 39097: // Magic Sandglass
            case 34164: // Escape Button
            case 34163: // Teleport Scroll
            case 34168: // Time Mantle
            case 25604: // Fast Jet
            case 34180: // Stopwatch; the named multi-use items (coffee packs, Tear Plier, sandglasses, ...)
                return true;
            default: // any other item: multi-use only by sub type 130, 135 or 140
                return item.subType == 135 || item.subType == 140 || item.subType == 130; // multi-use item sub types
        }
    }

    std::string hintSuffix(const formats::ItemRecord& record, const protocol::ItemInstance& item) {
        const int wear = item.wear;
        const auto times = [](int count) { return "(Remaining times:" + std::to_string(count) + "Times)"; };
        const auto ceilTimes = [&] { return static_cast<int>(std::ceil((kWornOut - wear) / (static_cast<double>(kWornOut) / record.maxUses))); };
        const int period = usePeriod(record);
        if (period != 0) {
            // 24 hours per day (the expiry is a TDateTime in days)
            const double hoursLeft = (item.expiry - core::nowAsDateTime()) * 24;
            int days = static_cast<int>(std::floor(period + hoursLeft)) / 24;
            int hours = static_cast<int>(std::floor(period + hoursLeft)) % 24;
            std::string dayText;
            std::string hourText;
            if (days > 0) {
                if (period + hoursLeft > period) {
                    days = period / 24;
                    hours = period % 24;
                }
                dayText = std::to_string(days) + " Day ";
            }
            if (hours > 0) {
                hourText = std::to_string(hours) + " Hour ";
            }
            if (days <= 0 && hours <= 0) {
                hourText = "In one hour";
            }
            return "(Time:" + dayText + hourText + ")";
        }
        if (record.subType == 135) { // item sub type 135: multi-use item
            return times((kWornOut - wear) / 25); // 25 wear points per use
        }
        if (record.maxUses != 0) {
            const int step = kWornOut / record.maxUses;
            switch (record.id) {
                case 25221: // Coffee Pack
                case 25620: // Super Caffeine Pack
                case 39074: // Magic Sandglass
                case 39097: // Magic Sandglass; named multi-use items counted by floor division
                    return times((kWornOut - wear) / step);
                case 34164: // Escape Button
                case 34152: // Tear Plier
                case 34163: // Teleport Scroll
                case 25604: // Fast Jet
                case 34168: // Time Mantle
                case 34180: // Stopwatch; named multi-use items counted by ceiling division
                    return times(ceilTimes());
                default: // any other item id: fall through to the sub type checks
                    break;
            }
            if (record.subType == 130 || record.subType == 140) { // item sub types 130 and 140: multi-use items
                return times((kWornOut - wear) / step);
            }
        }
        if (record.id == 34132) { // item id 34132: Loudspeaker, multi-use item
            return times((kWornOut - wear) / 5); // 5 wear points per use
        }
        if (record.equipSlot >= 1 && record.equipSlot <= 5 && item.fusionLevel > 0) { // equip slots 1..5: worn gear that takes fusion
            return "+" + std::to_string(item.fusionLevel);
        }
        return {};
    }

    std::string stallItemName(const formats::ItemTable& items, const protocol::ItemInstance& item) {
        const auto found = items.find(item.itemId);
        if (!found) {
            return {};
        }
        const int bottle = bottleKind(*found);
        const bool worn = found->equipSlot >= 1 && found->equipSlot <= 5; // equip slots 1..5: worn gear that takes fusion
        if (bottle != 1 && bottle != 2 && worn && item.fusionLevel > 0) {
            return found->name + "+" + std::to_string(item.fusionLevel);
        }
        return found->name;
    }

    std::string describeStallItem(const formats::ItemTable& items, const protocol::ItemInstance& item, double now) {
        constexpr std::size_t kLines = 10; // number of info lines (1..10) in the original's ShowLine
        constexpr std::size_t kLongestDescription = 66; // description cut to 66 characters
        const auto found = items.find(item.itemId);
        if (!found) {
            return {};
        }
        const formats::ItemRecord& record = *found;
        const bool equipment = record.equipSlot >= 1 && record.equipSlot <= 6; // equip slots 1..6: gear
        const bool boxed = slot9Kind(record) == 2 && item.boxItemId != 0; // slot 9 kind 2: box item
        // line numbers of the original's ShowLine: 1 attributes, 3 level, 4 times, 5 status, 6 insert, 7 time left, 8 seal resistance, 9 box, 10 description
        std::array<bool, kLines + 1> shows{};
        shows[1] = shows[2] = shows[3] = true;
        shows[4] = isMultiUse(record);
        shows[5] = (equipment && !shows[4]) || boxed;
        shows[6] = equipment;
        shows[7] = usePeriod(record) != 0;
        shows[8] = record.subType == 145; // item sub type 145: seal-resistance item
        shows[9] = boxed;
        shows[10] = (equipment && record.subType == 138) || shows[4] || shows[7]; // item sub type 138: gear shown with its description

        std::array<std::string, kLines + 1> lines{};
        const int bottle = bottleKind(record);
        if (bottle == 1 || bottle == 2) {
            const int capacity = record.attributeValues[0] - kNeutralValue;
            lines[1] = std::string(bottle == 1 ? "HP: " : "SP: ") + std::to_string(capacity - item.usedAmount) + "/" + std::to_string(capacity);
        } else {
            lines[1] = stallAttributes(items, record, item);
        }
        if (record.requiredLevel != 0) {
            lines[3] = "Obtained at:" + std::to_string(record.requiredLevel);
        }
        if (shows[4]) {
            const double perUse = static_cast<double>(kWornOut) / record.maxUses;
            lines[4] = "(Times: " + std::to_string(static_cast<int>(std::ceil((kWornOut - item.wear) / perUse))) + "Times)";
        }
        if (shows[9]) {
            const auto box = items.find(item.boxItemId);
            lines[9] = "Items inside the box:" + (box ? box->name : std::string());
        }
        if (shows[5]) {
            lines[5] = stallWearStatus(item.wear);
            if (shows[6] && item.insertItemId != 0 && isGem(items, item.insertItemId)) {
                lines[6] = "insert:" + items.find(item.insertItemId)->name;
            }
        }
        if (shows[7]) {
            const int period = usePeriod(record);
            // 24 hours per day (the expiry is a TDateTime in days)
            const double hoursLeft = (item.expiry - now) * 24;
            int days = static_cast<int>(std::floor(period + hoursLeft)) / 24;
            int hours = static_cast<int>(std::floor(period + hoursLeft)) % 24;
            std::string dayText;
            std::string hourText;
            if (days > 0) {
                if (period + hoursLeft > period) {
                    days = period / 24;
                    hours = period % 24;
                }
                dayText = std::to_string(days) + " Day ";
            }
            if (hours > 0) {
                hourText = std::to_string(hours) + " Hour ";
            }
            if (days <= 0 && hours <= 0) {
                hourText = period + hoursLeft > 0 ? "In one hour" : "Destroyed";
            }
            lines[7] = "(Time:" + dayText + hourText + ")";
        }
        if (shows[8]) {
            lines[8] = "(Resist seal effect:" + std::to_string(record.attributeValues[0] - kNeutralValue) + "%)";
        }
        lines[10] = record.description.substr(0, kLongestDescription);
        if (shows[9]) {
            lines[5] = lines[9] + " " + lines[5];
            lines[9].clear();
        }
        std::string text;
        for (std::size_t k = 1; k <= kLines; ++k) {
            if (!shows[k] || lines[k].empty()) {
                continue;
            }
            if (text.empty()) {
                text = lines[k];
            } else if (k == 4 || k == 7) { // lines 4 (times) and 7 (time left) go in front
                text = lines[k] + " " + text;
            } else {
                text += " " + lines[k];
            }
        }
        // item types 20..22, 37 and 38 show only the description
        const bool plain = (record.type >= 20 && record.type <= 22) || record.type == 37 || record.type == 38;
        return text.empty() || plain ? lines[10] : text;
    }

    std::vector<std::string> describeItem(const formats::ItemTable& items, const protocol::ItemInstance& item, bool wrapDescription, const BottlePools& pools) {
        std::vector<std::string> lines;
        const auto found = items.find(item.itemId);
        if (!found) {
            return lines;
        }
        const formats::ItemRecord& record = *found;
        const bool equipment = record.equipSlot >= 1 && record.equipSlot <= 6; // equip slots 1..6: gear
        const bool showsAttributes = bottleKind(record) == 0;
        const bool multiUse = isMultiUse(record);

        std::string plus;
        int fusionBonus = 0;
        if (showsAttributes && record.equipSlot >= 1 && record.equipSlot <= 5 && item.fusionLevel > 0) { // equip slots 1..5: worn gear that takes fusion
            plus = "+" + std::to_string(item.fusionLevel);
            int raised = 0;
            for (std::size_t k = 0; k < 2; ++k) { // two attribute slots per item
                if (fusionRaises(record.attributeKinds[k]) && record.attributeValues[k] > kNeutralValue) {
                    ++raised;
                }
            }
            fusionBonus = raised == 1 ? item.fusionLevel * 2 : raised == 2 ? item.fusionLevel : 0;
        }
        std::array<int, 2> adjust{}; // one adjust per attribute slot
        if (item.attributeAdjust > 0) {
            adjust[0] = signedNibble(item.attributeAdjust);
            adjust[1] = signedNibble((item.attributeAdjust & 0x0F) << 4); // low nibble moved to the high nibble
        }

        lines.push_back("[" + record.name + plus + "]");
        if (record.type == kQuestItemType) {
            lines.push_back("<Quest prop>");
        } else if ((record.flags & kNoTradeFlag) != 0 || item.lookItemId != 0) {
            lines.push_back("<Can't be transfered>");
        }
        if (showsAttributes) {
            for (std::size_t k = 0; k < 2; ++k) { // two attribute slots per item
                const int kind = record.attributeKinds[k];
                const int value = record.attributeValues[k];
                if (kind == kHiddenAttribute || value == 0 || value == kNeutralValue) {
                    continue;
                }
                std::string line = attributeName(kind);
                if (!line.empty()) {
                    line += ": ";
                }
                const int shown = value - kNeutralValue + adjust[k];
                line += (shown >= 0 ? "+" : "") + std::to_string(shown);
                // sub type 113 exp items (kind 36) and sub type 119 (kind 100, shoot resistance) show a percent
                if ((record.subType == 113 && kind == 36) || (record.subType == 119 && kind == 100)) {
                    line += "%";
                }
                line += " ";
                if (fusionBonus > 0 && value > kNeutralValue) {
                    line += "+" + std::to_string(fusionBonus);
                }
                lines.push_back(line);
            }
        }
        lines.push_back("Item rank: " + std::to_string(record.rank));
        if (record.requiredLevel != 0) {
            lines.push_back("Obtained at: " + std::to_string(record.requiredLevel));
        }
        const int bottle = bottleKind(record);
        if (bottle == 1 || bottle == 2) {
            const int capacity = record.attributeValues[0] - kNeutralValue;
            lines.push_back(std::to_string(capacity - item.usedAmount) + "/" + std::to_string(capacity));
        } else if (bottle == 3 || bottle == 4) { // bottle kinds 3 and 4: the special bottles that show the player's pool
            constexpr int kMostFill = 2000000; // capacity shown for the special HP/SP bottles: 2,000,000
            lines.push_back(std::to_string(bottle == 3 ? pools.hp : pools.sp) + "/" + std::to_string(kMostFill));
        }
        if (usePeriod(record) != 0) {
            const int period = usePeriod(record);
            // 24 hours per day (the expiry is a TDateTime in days)
            const double hoursLeft = (item.expiry - core::nowAsDateTime()) * 24;
            int days = static_cast<int>(std::floor(period + hoursLeft)) / 24;
            int hours = static_cast<int>(std::floor(period + hoursLeft)) % 24;
            std::string dayText;
            std::string hourText;
            if (days > 0) {
                if (period + hoursLeft > period) {
                    days = period / 24;
                    hours = period % 24;
                }
                dayText = std::to_string(days) + " Day ";
            }
            if (hours > 0) {
                hourText = " " + std::to_string(hours) + " Hour ";
            }
            if (days <= 0 && hours <= 0) {
                hourText = "In one hour";
            }
            lines.push_back("Time: " + dayText + hourText);
        }
        if (multiUse) {
            const double perUse = static_cast<double>(kWornOut) / record.maxUses;
            lines.push_back("Times: " + std::to_string(static_cast<int>(std::ceil((kWornOut - item.wear) / perUse))) + "Times");
        }
        if (equipment && !multiUse) {
            lines.push_back(wearStatus(item.wear));
        }
        if (equipment && item.insertItemId != 0) {
            if (const auto stone = items.find(item.insertItemId)) {
                lines.push_back("insert: " + stone->name);
            }
        }
        if (item.lookItemId != 0) {
            if (const auto look = items.find(item.lookItemId)) {
                lines.push_back("Pose: " + look->name);
            }
        }
        lines.push_back(wrapDescription ? ui::wrapTextLines(record.description, 22) : record.description); // popup line width in characters
        return lines;
    }

}
