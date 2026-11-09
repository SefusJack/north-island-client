#include "client/models/character/StatusEffects.h"

#include <cstdio>

namespace client {

    namespace {

        /// @brief Formats minutes as "H Hour M Minute ", leaving the hours out when 0.
        /// @details Original: IntToStr(hours) + ' Hour ' + IntToStr(minutes) + ' Minute '.
        /// @param minutes Total minutes.
        /// @return The text.
        std::string hoursAndMinutes(int minutes) {
            const int hours = minutes / 60; // 60 minutes per hour
            const int rest = minutes - hours * 60; // 60 minutes per hour
            if (hours != 0) {
                return std::to_string(hours) + " Hour " + std::to_string(rest) + " Minute ";
            }
            return std::to_string(rest) + " Minute ";
        }

        /// @brief Formats a rate given in tenths as a decimal number.
        /// @details Original: FloatToStr(rate / 10).
        /// @param rate Rate times 10.
        /// @return The rate as text, e.g. "1.5".
        std::string tenths(int rate) {
            char text[32]; // text buffer size, chars
            std::snprintf(text, sizeof text, "%g", rate / 10.0); // rate is in tenths
            return text;
        }

    }

    std::string StatusEffects::doubleExp(int kind, std::optional<int> value) {
        m_doubleExpActive = kind;
        if (!value) {
            return {};
        }
        if (kind == 1) { // 5/14 kind 1: on
            if (*value == 0) {
                return "The 2x EXP is at the upper limit\xA1I";
            }
            return "The rest time of double EXP\xA1G" + hoursAndMinutes(*value);
        }
        if (kind == 2 && *value == 1) { // 5/14 kind 2: refused, value 1
            return "Cannot get double EXP\xA1I";
        }
        return {};
    }

    std::string StatusEffects::expPotionIcon(int active, int rate) {
        m_doubleExpActive = active;
        m_expPotionRate = rate;
        return {};
    }

    std::string StatusEffects::expPotionRefused(int reason) {
        if (reason == 1) { // 5/17 2 reason 1: once only
            return "Can be used only once";
        }
        if (reason == 2) { // 5/17 2 reason 2: against a Statue Voucher
            return "Cant' use  Statue Voucher when you use EXP Potion ";
        }
        return {};
    }

    std::string StatusEffects::expPotionRemaining(int active, int minutes, int rate) {
        m_doubleExpActive = active;
        m_expPotionRate = rate;
        return tenths(rate) + " Effect of EXP Potion Remained: " + hoursAndMinutes(minutes);
    }

    std::string StatusEffects::expPotionExpired(int active) {
        m_doubleExpActive = active;
        m_expPotionRate = 0;
        return "The date of the EXP is expired";
    }

    std::string StatusEffects::luckyGrass(int kind, int minutes) {
        switch (kind) {
            case 0: // 5/15 kind 0: off
                m_luckyGrassOn = false;
                return {};
            case 1: // 5/15 kind 1: on (minutes left)
                m_luckyGrassOn = true;
                return "Rest time for using lucky grass\xA1G" + hoursAndMinutes(minutes);
            case 2: // 5/15 kind 2: not in effect
                m_luckyGrassOn = false;
                return "Lucky grass isn't on effect";
            case 3: // 5/15 kind 3: ending
                return "Time of using lucky grass is ending";
            case 4: // 5/15 kind 4: the time left
                return "Rest time for using lucky grass\xA1G" + std::to_string(minutes) + " Silence";
            default: // unknown kind: no line
                return {};
        }
    }

    std::string StatusEffects::spaBuns(int kind, std::optional<int> value) {
        m_spaBuns = kind;
        if (!value) {
            return {};
        }
        if (kind == 1) { // 5/16 kind 1: on (minutes left)
            return "The rest time of using spa buns\xA1G" + std::to_string(*value) + " Minute ";
        }
        if (kind == 2) { // 5/16 kind 2: refused (value 0: sauna bag at its upper limit)
            return *value == 0 ? "The sauna bag has reached upper limit\xA1I" : "Cannot use spa buns\xA1I";
        }
        return {};
    }

    std::string StatusEffects::expPause(int sub, bool paused, const std::string& itemName) {
        m_doubleExpPaused = paused;
        if (sub != 2) { // 23/204 sub 2: the Stopwatch was used or cancelled
            return {};
        }
        if (paused) {
            return "Use " + itemName + " The double exp function is paused";
        }
        return "Cancel " + itemName + " Double exp state continues";
    }

    StatusEffects::TrainingNotice StatusEffects::trainingItem(
        int kind,
        int effect,
        int minutes,
        bool sound,
        bool fighting,
        const std::string& itemName
    ) {
        const std::string name = m_trainingItem == 1 ? itemName : std::string();
        TrainingNotice notice;
        switch (kind) {
            case 1: { // 23/106 kind 1: the effect runs
                if (effect != 1) {
                    break;
                }
                m_trainingItem = effect;
                // 60 minutes per hour, 24 hours per day
                const int days = minutes / 60 / 24;
                const int hours = minutes / 60 % 24;
                const int mins = minutes % 60;
                std::string left;
                if (days > 0) {
                    left = std::to_string(days) + " Day ";
                }
                if (hours > 0) {
                    left += std::to_string(hours) + " Hour ";
                }
                if (mins > 0) {
                    left += std::to_string(mins) + " Minute ";
                }
                if (left.empty()) {
                    break;
                }
                if (fighting) {
                    notice.text = itemName + " Effective Time\241G" + left;
                } else {
                    notice.text = itemName + " Effective Time\241G" + left + "\rTeleport to Training Island ?";
                    notice.askTeleport = true;
                }
                notice.sound = sound;
                break;
            }
            case 2: // 23/106 kind 2: the effect is over
                m_trainingItem = 0;
                notice.text = name + " time's up, State cancelled";
                break;
            case 3: // 23/106 kind 3: one is already running
                notice.text = "Already Exist " + name + " State, can't be used";
                break;
            case 4: // 23/106 kind 4: the time is at its limit
                notice.text = name + " Effective time has reached upper limit, can't be used.";
                break;
            default: // unknown kind: no notice
                break;
        }
        return notice;
    }

    namespace {

        // names of the five buffs: ATK, DEF, MATK, MDEF, SPD
        constexpr std::array<const char*, 5> kStrengthNames = {"Strengthen ATK", "Strengthen DEF", "Strengthen MATK", "Strengthen MDEF", "Strengthen SPD"};

        /// @brief Reads the 4-bit field of a stat from the buff mask.
        /// @details Original: GetBitField(FStrengthenBuffMask, 4, stat + 2).
        /// @param mask The buff mask.
        /// @param stat Stat 1..5.
        /// @return The level nibble.
        int nibble(std::uint32_t mask, int stat) {
            return static_cast<int>((mask >> (4 * (stat + 1))) & 15);
        } // a nibble is 4 bits, per stat from the third nibble; mask 15 = 4 bits

    }

    int StatusEffects::strengthLevel(int stat) const { return nibble(m_strengthMask, stat); }

    int StatusEffects::strengthValue(int stat, int level) {
        // stats 1..4 (ATK to MDEF) and levels 1..2 have a value
        if (stat < 1 || stat > 4 || level < 1 || level > 2) {
            return 0;
        }
        return level == 1 ? 300 : 150; // buff amount shown for level 1 / level 2 (stat points)
    }

    std::string StatusEffects::strengthBuffTimes(const std::array<int, 5>& minutes) const {
        std::string line;
        for (int stat = 1; stat <= 5; ++stat) { // five buffs
            const int level = strengthLevel(stat);
            const int left = minutes[static_cast<std::size_t>(stat - 1)];
            if (level < 1 || level > 5 || left <= 0) { // levels 1..5
                continue;
            }
            // 60 minutes per hour, 24 hours per day
            const int days = left / 60 / 24;
            const int hours = left / 60 % 24;
            const int mins = left % 60;
            std::string time;
            if (days != 0) {
                time += std::to_string(days) + " Day ";
            }
            if (hours != 0) {
                time += std::to_string(hours) + " Hour ";
            }
            if (mins != 0) {
                time += std::to_string(mins) + " Minute ";
            }
            if (!line.empty()) {
                line += "\r";
            }
            line += std::string(kStrengthNames[static_cast<std::size_t>(stat - 1)]) + "+" + std::to_string(strengthValue(stat, level)) + ": " + time;
        }
        return line;
    }

    std::vector<std::string> StatusEffects::strengthBuffMask(std::uint32_t mask) {
        std::vector<std::string> lines;
        if (m_strengthMask > mask) {
            const std::uint32_t ran = m_strengthMask ^ mask;
            for (int stat = 1; stat <= 5; ++stat) { // five buffs
                if (nibble(ran, stat) != 0) {
                    lines.push_back(std::string(kStrengthNames[static_cast<std::size_t>(stat - 1)]) + " Out of effective time");
                }
            }
        }
        m_strengthMask = mask;
        return lines;
    }

    std::string StatusEffects::bodyShift(int state) {
        switch (state) {
            case 0: // 23/207 state 0: time up
                m_bodyShift = 0;
                return "Cancel body shifting, because time is up";
            case 1: // 23/207 state 1: changed
                m_bodyShift = 1;
                return "Changed successfully";
            case 2: // 23/207 state 2: off
                m_bodyShift = 2;
                return "Cancel body shifting";
            case 3: // 23/207 state 3: off silently
                m_bodyShift = 2;
                return {};
            default: // unknown state: no line
                return {};
        }
    }

    std::string StatusEffects::bodyShiftRefusal(int reason) {
        switch (reason) {
            case 1: // 23/207 3 reason 1: already changed
                return "Already changed, can't use";
            case 2: // 23/207 3 reason 2: repeated use
                return "Can't use repeatedly";
            case 3: // 23/207 3 reason 3: cannot transform
                return "Can't transform.";
            case 4: // 23/207 3 reason 4: currently shifting body
                return "Can't use when you are shifting body";
            default: // unknown reason: no line
                return {};
        }
    }

    std::string StatusEffects::bodyShiftAsk(int minutes) const {
        if (minutes == 0) {
            return {};
        }
        // 60 minutes per hour
        return "Remaining time of body shifting: " + std::to_string(minutes / 60) + " Hour " + std::to_string(minutes % 60) + " minute" +
               (m_bodyShift == 1 ? ", Cancel body shifting" : ", body shifting or not");
    }

    std::string StatusEffects::expHint() const {
        if (m_expPotionRate != 0) {
            return tenths(m_expPotionRate) + "X Exp";
        }
        return "2X Exp";
    }

    int StatusEffects::expIconItem() const {
        // rates are x10: 15..60 in steps of 5 map to item 34107 (1.5X EXP Potion) up to 34116 (6X EXP Potion)
        if (m_expPotionRate >= 15 && m_expPotionRate <= 60 && m_expPotionRate % 5 == 0) {
            return 34107 + (m_expPotionRate - 15) / 5;
        }
        if (m_expPotionRate == 80) { // rate 8x
            return 34123; // 8X EXP Potion
        }
        if (m_expPotionRate == 100) { // rate 10x
            return 34173; // 10X EXP Potion
        }
        return 25150; // Double Exp Pill item id (item table name: Smile Mud Statue)
    }

}
