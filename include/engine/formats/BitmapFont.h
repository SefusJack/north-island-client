#ifndef NORTH_ISLAND_FORMATS_BITMAP_FONT_H
#define NORTH_ISLAND_FORMATS_BITMAP_FONT_H

#include "core/Bytes.h"

#include <cstdint>
#include <filesystem>
#include <optional>

namespace formats {

    /// @brief The game's 15-pixel bitmap font, holding half-width (8 x 15) and full-width Big5 (16 x 15) glyph bitmaps.
    /// @details Original: font\tatpc1.twn (Traditional Chinese) or tatpc1.jpc (Simplified), every byte XOR 0x58. Half-width glyphs: single bytes 0..255, 15 bytes each from offset 0. Full-width glyphs: 30 bytes each (two bytes per row), glyph index < 0x3326 from offset 0xF00, the rest from offset 0x60D74. The leftmost pixel of a row is the most significant bit.
    class BitmapFont {
        public:
            static constexpr int kHalfWidth = 8; // half-width glyph width, px
            static constexpr int kFullWidth = 16; // full-width glyph width, px
            static constexpr int kHeight = 15; // glyph height, px
            static constexpr int kFullGlyphCount = 0x3656; // number of full-width (Big5) glyphs in the font file

            /// @brief Reads a font file and removes its XOR.
            /// @param path Font file.
            /// @return The font, nothing when unreadable.
            static std::optional<BitmapFont> load(const std::filesystem::path& path);

            /// @brief Tells whether a pixel of a half-width glyph is set.
            /// @param code Single-byte character code.
            /// @param x Column, 0..7.
            /// @param y Row, 0..14.
            /// @return True when set, false when set off or out of range.
            bool halfPixel(std::uint8_t code, int x, int y) const;

            /// @brief Tells whether a pixel of a full-width glyph is set.
            /// @param glyphIndex Glyph index (see big5Glyph).
            /// @param x Column, 0..15.
            /// @param y Row, 0..14.
            /// @return True when set, false when set off or out of range.
            bool fullPixel(int glyphIndex, int x, int y) const;

            /// @brief Finds the full-width glyph of a Big5 character in the font's order (frequent characters first, then less frequent ones, the symbols and the C6 A1.. block).
            /// @param lead Lead byte.
            /// @param trail Trail byte.
            /// @return The glyph index, nothing when the pair is not one of the font's characters (then both bytes are drawn as half-width glyphs).
            static std::optional<int> big5Glyph(std::uint8_t lead, std::uint8_t trail);

        private:
            /// @brief Reads one bit of a glyph row byte, leftmost pixel first.
            /// @param offset Byte offset in the font data.
            /// @param x Pixel within the byte, 0..7.
            /// @return True when set, false when set off or beyond the data.
            bool bit(std::size_t offset, int x) const;

            core::Bytes m_data;
    };

}

#endif
