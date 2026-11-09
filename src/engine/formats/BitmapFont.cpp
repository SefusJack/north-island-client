#include "formats/BitmapFont.h"

#include "core/FileSystem.h"

namespace formats {

    namespace {

        constexpr std::uint8_t kXorKey = 0x58; // every byte of the font file is XORed with this key
        constexpr std::size_t kFirstBankOffset = 0xF00; // file offset of the first full-width glyph bank (glyphs 0..0x3325), after the half-width glyphs
        constexpr std::size_t kSecondBankOffset = 0x60D74; // file offset of the second full-width glyph bank (glyphs from kSecondBankStart)
        constexpr int kSecondBankStart = 0x3326; // first glyph index stored in the second bank

    }

    std::optional<BitmapFont> BitmapFont::load(const std::filesystem::path& path) {
        auto data = core::readFile(path);
        if (!data) {
            return std::nullopt;
        }
        BitmapFont font;
        font.m_data = std::move(*data);
        for (std::uint8_t& byte : font.m_data) {
            byte ^= kXorKey;
        }
        return font;
    }

    bool BitmapFont::bit(std::size_t offset, int x) const {
        if (offset >= m_data.size()) {
            return false;
        }
        return (m_data[offset] & (0x80 >> x)) != 0; // leftmost pixel is the most significant bit (0x80)
    }

    bool BitmapFont::halfPixel(std::uint8_t code, int x, int y) const {
        if (x < 0 || x >= kHalfWidth || y < 0 || y >= kHeight) {
            return false;
        }
        return bit(static_cast<std::size_t>(code) * kHeight + static_cast<std::size_t>(y), x);
    }

    bool BitmapFont::fullPixel(int glyphIndex, int x, int y) const {
        if (glyphIndex < 0 || glyphIndex >= kFullGlyphCount || x < 0 || x >= kFullWidth || y < 0 || y >= kHeight) {
            return false;
        }
        // a full-width glyph takes 30 bytes (15 rows of 2 bytes)
        const std::size_t base = glyphIndex < kSecondBankStart ? kFirstBankOffset + static_cast<std::size_t>(glyphIndex) * 30
                                                               : kSecondBankOffset + static_cast<std::size_t>(glyphIndex - kSecondBankStart) * 30;
        return bit(base + static_cast<std::size_t>(y) * 2 + static_cast<std::size_t>(x / 8), x % 8); // 2 bytes per row; 8 pixels per byte
    }

    std::optional<int> BitmapFont::big5Glyph(std::uint8_t lead, std::uint8_t trail) {
        if (lead < 0x80) { // below 0x80 is a single-byte character
            return std::nullopt;
        }
        // Big5 trail bytes run 0x40..0x7E then 0xA1..0xFE (157 per lead byte); 0x62 closes the 0x7F..0xA0 gap
        const auto column = [](int t) { return t > 0x7E ? t - 0x62 : t - 0x40; };
        int index = 0;
        // lead 0xA1..0xA3: symbols after the 0x3326 frequent block; 0xA4..0xC6: frequent characters; 0xC6 0xA1..0xC8: 94 trails per lead from glyph 0x34BE; 0xC9..: less frequent from glyph 0x1519
        if (lead < 0xA4) {
            index = (lead - 0xA1) * 157 + column(trail) + kSecondBankStart;
        } else if (lead < 0xC6 || (lead == 0xC6 && trail < 0xA1)) {
            index = (lead - 0xA4) * 157 + column(trail);
        } else if (lead < 0xC9) {
            index = (lead - 0xC6) * 94 + trail - 0xA1 + 0x34BE;
        } else {
            index = (lead - 0xC9) * 157 + 0x1519 + column(trail);
        }
        if (index < 0 || index >= kFullGlyphCount) {
            return std::nullopt;
        }
        return index;
    }

}
