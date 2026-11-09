#ifndef NORTH_ISLAND_FORMATS_COLOR_PATTERN_H
#define NORTH_ISLAND_FORMATS_COLOR_PATTERN_H

#include "core/Color.h"

#include <array>
#include <cstdint>

namespace formats {

    /// @brief A character's colours: 45 digits (4 = unchanged) that brighten or darken blocks of 16 palette entries of its sprites, (digit - 4) * 25 per channel.
    /// @details Original: pattern codes are nine-digit numbers sent by the server and stored in Item.dat; apply() copies their digits into the pattern by part (DZ_Human.SeparatePatt). Part 1: digits 0-8; part 2: 9-17; part 3: 18-26; part 4: 27-35; part 5: 9-11, 39-41, 42-44. An item only writes the groups its equipment slot may change (mask); mask 255 writes all.
    class ColorPattern {
        public:
            static constexpr std::size_t kSize = 45; // number of digits in a pattern
            static constexpr int kAllGroups = 255; // mask with all 8 groups set: the item may write every digit group

            /// @brief Makes a pattern with every digit unchanged (4).
            ColorPattern() { m_digits.fill(4); } // digit 4 = colour unchanged

            /// @brief Makes a pattern from its digits.
            /// @param digits The 45 digits.
            explicit ColorPattern(
                const std::array<std::uint8_t, kSize>& digits
            )
                : m_digits(digits) {}

            /// @brief Copies the digits of a pattern code into the pattern by part.
            /// @param code Nine-digit pattern code.
            /// @param part Character part, 1..5.
            /// @param mask Equipment slot allowed to write a group; 255 writes all.
            void apply(std::int32_t code, int part, int mask = kAllGroups);

            /// @brief Builds the code that apply(code, part) would turn into the pattern's digits of that part.
            /// @param part Character part, 1..5.
            /// @return The code, 0 for a part out of range.
            std::int32_t code(int part) const;

            /// @brief Gives one digit.
            /// @param index Digit index, 0..44.
            /// @return The digit.
            std::uint8_t digit(std::size_t index) const { return m_digits[index]; }

            /// @brief Sets one digit.
            /// @param index Digit index, 0..44.
            /// @param value New digit.
            void setDigit(std::size_t index, std::uint8_t value) { m_digits[index] = value; }

            /// @brief Applies the pattern to a palette; face sprites only take the hair, skin and the 208..223 block.
            /// @param palette The 256-entry palette.
            /// @param faceSprite True for a face sprite.
            /// @return The tinted palette.
            std::array<core::Color, 256> tint(const std::array<core::Color, 256>& palette, bool faceSprite) const; // 256 = palette entries

            /// @brief Gives all digits.
            /// @return The 45 digits.
            const std::array<std::uint8_t, kSize>& digits() const { return m_digits; }

            /// @brief Compares the digits.
            /// @param other Pattern to compare with.
            /// @return True when equal.
            bool operator==(const ColorPattern& other) const { return m_digits == other.m_digits; }

        private:
            /// @brief Writes a three-digit group (0..999) into three consecutive digits.
            /// @param group The group value.
            /// @param first Index of the first digit written.
            void writeGroup(int group, std::size_t first);

            std::array<std::uint8_t, kSize> m_digits{};
    };

}

#endif
