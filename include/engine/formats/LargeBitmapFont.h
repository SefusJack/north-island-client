#ifndef NORTH_ISLAND_FORMATS_LARGE_BITMAP_FONT_H
#define NORTH_ISLAND_FORMATS_LARGE_BITMAP_FONT_H

#include "core/Bytes.h"

#include <cstdint>
#include <filesystem>
#include <optional>

namespace formats {

    /// @brief The game's 24-pixel bitmap font.
    /// @details Original: font\tatpc2.twn (Traditional Chinese) or tatpc2.jpc (Simplified); LoadChinFonts, DrawLargeHalfGlyph, DrawLargeFullGlyph; every byte XOR 0x58.
    /// Half-width glyphs (single bytes 0..255): 16 bits a row, 24 rows (48 bytes), from offset 0. Full-width glyphs (Big5 pairs, numbering of BitmapFont::big5Glyph):
    /// 24 x 24, 3 bytes a row (72 bytes), glyph index < 0x3326 from offset 0x3000, the rest from offset 0xE92B0. The leftmost pixel of a row is the most significant bit.
    class LargeBitmapFont {
        public:
            static constexpr int kHalfWidth = 16; // half-width glyph width, px
            static constexpr int kFullWidth = 24; // full-width glyph width, px
            static constexpr int kHeight = 24; // glyph height, px

            /// @brief Loads and decodes a font file.
            /// @param path Font file path.
            /// @return The font, or nothing when the file cannot be read.
            static std::optional<LargeBitmapFont> load(const std::filesystem::path& path);

            /// @brief Whether a pixel of a half-width glyph is set.
            /// @param code Single-byte character code.
            /// @param x Column (0 is leftmost).
            /// @param y Row (0 is top).
            /// @return True when the pixel is set.
            bool halfPixel(std::uint8_t code, int x, int y) const;

            /// @brief Whether a pixel of a full-width glyph is set.
            /// @param glyphIndex Big5 glyph number.
            /// @param x Column (0 is leftmost).
            /// @param y Row (0 is top).
            /// @return True when the pixel is set.
            bool fullPixel(int glyphIndex, int x, int y) const;

        private:
            /// @brief Reads one bit of a glyph row.
            /// @param offset Byte offset of the row start in the data.
            /// @param x Column (0 is the most significant bit).
            /// @return True when the bit is set.
            bool bit(std::size_t offset, int x) const;

            core::Bytes m_data;
    };

}

#endif
