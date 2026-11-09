#ifndef NORTH_ISLAND_CORE_COLOR_H
#define NORTH_ISLAND_CORE_COLOR_H

#include <cstdint>
#include <optional>
#include <string>

namespace core {

    /// @brief An 8-bit-per-channel colour with alpha.
    struct Color {
        std::uint8_t r = 0;
        std::uint8_t g = 0;
        std::uint8_t b = 0;
        std::uint8_t a = 255; // alpha 255 = fully opaque

        /// @brief Makes an opaque colour from 0xRRGGBB.
        /// @param rgb Packed colour.
        /// @return The colour.
        static Color fromRgb(std::uint32_t rgb);

        /// @brief Expands a 16-bit RGB565 value, as the game stores colours, by bit replication (as a 16-bit display mode shows it).
        /// @param value RGB565 value.
        /// @return The colour.
        static Color fromRgb565(std::uint16_t value);

        /// @brief Packs the colour into RGB565 (alpha dropped).
        /// @return The RGB565 value.
        std::uint16_t toRgb565() const;

        /// @brief Parses "#RRGGBB" or "#AARRGGBB".
        /// @param text The colour text.
        /// @return The colour, nothing when the text is malformed.
        static std::optional<Color> parse(const std::string& text);

        /// @brief Compares all four channels.
        /// @param other Colour to compare with.
        /// @return True when equal.
        bool operator==(const Color& other) const { return r == other.r && g == other.g && b == other.b && a == other.a; }

        /// @brief Compares all four channels.
        /// @param other Colour to compare with.
        /// @return True when different.
        bool operator!=(const Color& other) const { return !(*this == other); }
    };

}

#endif
