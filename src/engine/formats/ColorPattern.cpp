#include "formats/ColorPattern.h"

#include <algorithm>

namespace formats {

    namespace {

        /// @brief Where one digit group of a code goes and which equipment slot may write it.
        struct Placement {
            std::size_t first;
            int mask;
        };

        // rows = character parts 1..5, columns = the three digit groups of a code; each entry = {first digit index in the 45 digits, equipment slot allowed to write it (-1 = no item slot)}
        constexpr Placement kPlacement[5][3] = {
            {{0, 1}, {3, -1}, {6, -1}}, // part 1: digits 0-2 (slot 1), 3-5, 6-8
            {{9, -1}, {12, 2}, {15, 2}}, // part 2: digits 9-11, 12-14 (slot 2), 15-17 (slot 2)
            {{18, 2}, {21, 5}, {24, 5}}, // part 3: digits 18-20 (slot 2), 21-23 (slot 5), 24-26 (slot 5)
            {{27, 3}, {30, 3}, {33, 4}}, // part 4: digits 27-29 (slot 3), 30-32 (slot 3), 33-35 (slot 4)
            {{9, -1}, {39, -1}, {42, 5}}, // part 5: digits 9-11 (shared with part 2), 39-41, 42-44 (slot 5)
        };

        /// @brief Shifts a colour channel, keeping it away from pure black and white.
        /// @param value Channel value.
        /// @param delta Amount to add.
        /// @return The shifted channel.
        std::uint8_t tintChannel(std::uint8_t value, int delta) {
            return static_cast<std::uint8_t>(std::clamp(value + delta, 5, 250)); // channel limits 5..250
        }

    }

    void ColorPattern::writeGroup(int group, std::size_t first) {
        // hundreds, tens and ones digits of the group
        m_digits[first] = static_cast<std::uint8_t>(group / 100);
        m_digits[first + 1] = static_cast<std::uint8_t>((group / 10) % 10);
        m_digits[first + 2] = static_cast<std::uint8_t>(group % 10);
    }

    void ColorPattern::apply(std::int32_t code, int part, int mask) {
        if (part < 1 || part > 5) { // parts are numbered 1..5
            return;
        }
        // the nine-digit code holds three groups of three digits
        const int groups[3] = {code / 1000000, (code / 1000) % 1000, code % 1000};
        for (int g = 0; g < 3; ++g) {
            const Placement& placement = kPlacement[part - 1][g];
            if (mask == kAllGroups || mask == placement.mask) {
                writeGroup(groups[g], placement.first);
            }
        }
    }

    std::int32_t ColorPattern::code(int part) const {
        if (part < 1 || part > 5) { // parts are numbered 1..5
            return 0;
        }
        std::int32_t result = 0;
        for (const Placement& placement : kPlacement[part - 1]) {
            for (std::size_t i = 0; i < 3; ++i) { // three digits per group
                result = result * 10 + m_digits[placement.first + i]; // decimal digit shift
            }
        }
        return result;
    }

    std::array<core::Color, 256> ColorPattern::tint(const std::array<core::Color, 256>& palette, bool faceSprite) const {
        std::array<core::Color, 256> result = palette;
        const auto tintBlock = [&](std::size_t firstEntry, std::size_t firstDigit) {
            // (digit - 4) * 25 per channel: digit 4 is unchanged; a block is 16 palette entries
            const int dr = (m_digits[firstDigit] - 4) * 25;
            const int dg = (m_digits[firstDigit + 1] - 4) * 25;
            const int db = (m_digits[firstDigit + 2] - 4) * 25;
            for (std::size_t i = firstEntry; i < firstEntry + 16; ++i) {
                result[i].r = tintChannel(result[i].r, dr);
                result[i].g = tintChannel(result[i].g, dg);
                result[i].b = tintChannel(result[i].b, db);
            }
        };
        if (faceSprite) {
            // face sprite: palette entries 32 and 48 (hair, skin) take digits 3 and 6
            tintBlock(32, 3);
            tintBlock(48, 6);
        } else {
            // 12 blocks of 16 entries from entry 16, three digits per block
            for (std::size_t block = 0; block < 12; ++block) {
                tintBlock(16 + block * 16, block * 3);
            }
        }
        tintBlock(208, 39); // entries 208..223 take digits 39..41
        return result;
    }

}
