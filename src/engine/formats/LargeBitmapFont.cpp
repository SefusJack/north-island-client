#include "formats/LargeBitmapFont.h"

#include "core/FileSystem.h"
#include "formats/BitmapFont.h"

namespace formats {

    namespace {

        constexpr std::uint8_t kXorKey = 0x58; // every byte of the font file is XORed with this key
        constexpr std::size_t kHalfGlyphBytes = 48; // half-width glyph: 24 rows of 2 bytes
        constexpr std::size_t kFullGlyphBytes = 72; // full-width glyph: 24 rows of 3 bytes
        constexpr std::size_t kFirstBankOffset = 0x3000; // file offset of the first full-width glyph bank (glyphs 0..0x3325)
        constexpr std::size_t kSecondBankOffset = 0xE92B0; // file offset of the second full-width glyph bank (glyphs from kSecondBankStart)
        constexpr int kSecondBankStart = 0x3326; // first glyph index stored in the second bank

    }

    std::optional<LargeBitmapFont> LargeBitmapFont::load(const std::filesystem::path& path) {
        auto data = core::readFile(path);
        if (!data) {
            return std::nullopt;
        }
        LargeBitmapFont font;
        font.m_data = std::move(*data);
        for (std::uint8_t& byte : font.m_data) {
            byte ^= kXorKey;
        }
        return font;
    }

    bool LargeBitmapFont::bit(std::size_t offset, int x) const {
        if (offset >= m_data.size()) {
            return false;
        }
        return (m_data[offset] & (0x80 >> x)) != 0; // 0x80: most significant bit is the leftmost pixel
    }

    bool LargeBitmapFont::halfPixel(std::uint8_t code, int x, int y) const {
        if (x < 0 || x >= kHalfWidth || y < 0 || y >= kHeight) {
            return false;
        }
        return bit(
            static_cast<std::size_t>(code) * kHalfGlyphBytes + static_cast<std::size_t>(y) * 2 + static_cast<std::size_t>(x / 8),
            x % 8
        ); // 2 bytes per row; 8 pixels per byte
    }

    bool LargeBitmapFont::fullPixel(int glyphIndex, int x, int y) const {
        if (glyphIndex < 0 || glyphIndex >= BitmapFont::kFullGlyphCount || x < 0 || x >= kFullWidth || y < 0 || y >= kHeight) {
            return false;
        }
        const std::size_t base = glyphIndex < kSecondBankStart ? kFirstBankOffset + static_cast<std::size_t>(glyphIndex) * kFullGlyphBytes
                                                               : kSecondBankOffset + static_cast<std::size_t>(glyphIndex - kSecondBankStart) * kFullGlyphBytes;
        return bit(base + static_cast<std::size_t>(y) * 3 + static_cast<std::size_t>(x / 8), x % 8); // 3 bytes per row; 8 pixels per byte
    }

}
