#ifndef NORTH_ISLAND_FORMATS_FONT_ATLAS_H
#define NORTH_ISLAND_FORMATS_FONT_ATLAS_H

#include "core/Point.h"
#include "formats/BitmapFont.h"
#include "formats/Image.h"
#include "formats/LargeBitmapFont.h"

namespace formats {

    /// @brief How a bitmap font's glyphs lie in one picture, white ink on transparent: the 256 half-width glyphs side by side in the top row, then the full-width glyphs as many to a row as fit that width.
    /// @details The small font's texture is made this way, and an HD font pack has the same at twice the size each way (fonts\@2x\<font file>.png).
    class FontAtlasLayout {
        public:
            /// @brief Makes a layout for a font's glyph sizes.
            /// @param halfWidth Half-width glyph width.
            /// @param fullWidth Full-width glyph width.
            /// @param glyphHeight Glyph height.
            FontAtlasLayout(
                int halfWidth,
                int fullWidth,
                int glyphHeight
            )
                : m_halfWidth(halfWidth),
                  m_fullWidth(fullWidth),
                  m_glyphHeight(glyphHeight) {}

            /// @brief Gives the layout of the 15-pixel font (2048 wide).
            /// @return The layout.
            static FontAtlasLayout forSmallFont() { return {BitmapFont::kHalfWidth, BitmapFont::kFullWidth, BitmapFont::kHeight}; }

            /// @brief Gives the layout of the 24-pixel font (4096 wide).
            /// @return The layout.
            static FontAtlasLayout forLargeFont() { return {LargeBitmapFont::kHalfWidth, LargeBitmapFont::kFullWidth, LargeBitmapFont::kHeight}; }

            /// @brief Gives the atlas width: the 256 half-width glyphs side by side.
            /// @return Width in pixels.
            int width() const { return 256 * m_halfWidth; } // 256 half-width glyphs (one per byte value)

            /// @brief Gives the atlas height: the half-width row plus the full-width rows.
            /// @return Height in pixels.
            int height() const { return m_glyphHeight * ((BitmapFont::kFullGlyphCount + perRow() - 1) / perRow() + 1); }

            /// @brief Gives the half-width glyph width.
            /// @return Width in pixels.
            int halfWidth() const { return m_halfWidth; }

            /// @brief Gives the full-width glyph width.
            /// @return Width in pixels.
            int fullWidth() const { return m_fullWidth; }

            /// @brief Gives the glyph height.
            /// @return Height in pixels.
            int glyphHeight() const { return m_glyphHeight; }

            /// @brief Gives where a half-width glyph lies.
            /// @param code Single-byte character code.
            /// @return Top-left of the glyph.
            core::Point half(int code) const { return {code * m_halfWidth, 0}; }

            /// @brief Gives where a full-width glyph lies.
            /// @param index Glyph index (BitmapFont::big5Glyph's).
            /// @return Top-left of the glyph.
            core::Point full(int index) const { return {(index % perRow()) * m_fullWidth, (index / perRow() + 1) * m_glyphHeight}; }

            /// @brief Draws the atlas of a font with halfPixel(code, x, y) and fullPixel(index, x, y).
            /// @tparam Font Font type offering halfPixel and fullPixel.
            /// @param font The font to draw.
            /// @return The atlas picture.
            template <typename Font>
            Image draw(const Font& font) const {
                Image atlas(width(), height());
                const core::Color ink{255, 255, 255, 255}; // opaque white ink
                for (int code = 0; code < 256; ++code) { // 256 half-width glyphs
                    for (int y = 0; y < m_glyphHeight; ++y) {
                        for (int x = 0; x < m_halfWidth; ++x) {
                            if (font.halfPixel(static_cast<std::uint8_t>(code), x, y)) {
                                atlas.at(half(code).x + x, y) = ink;
                            }
                        }
                    }
                }
                for (int glyph = 0; glyph < BitmapFont::kFullGlyphCount; ++glyph) {
                    const core::Point at = full(glyph);
                    for (int y = 0; y < m_glyphHeight; ++y) {
                        for (int x = 0; x < m_fullWidth; ++x) {
                            if (font.fullPixel(glyph, x, y)) {
                                atlas.at(at.x + x, at.y + y) = ink;
                            }
                        }
                    }
                }
                return atlas;
            }

            /// @brief Scales each glyph's cell on its own so that no glyph takes in its neighbours' ink.
            /// @tparam Scale Callable that scales a picture to factor times its size each way.
            /// @param atlas The atlas to scale.
            /// @param factor Scale factor.
            /// @param scale The scaling callable.
            /// @return The scaled atlas.
            template <typename Scale>
            Image scaledCells(const Image& atlas, int factor, Scale scale) const {
                Image out(atlas.width() * factor, atlas.height() * factor);
                const auto scaleCell = [&](core::Point at, int width) {
                    Image cell(width, m_glyphHeight);
                    for (int y = 0; y < m_glyphHeight; ++y) {
                        for (int x = 0; x < width; ++x) {
                            cell.at(x, y) = atlas.at(at.x + x, at.y + y);
                        }
                    }
                    const Image big = scale(cell);
                    for (int y = 0; y < big.height(); ++y) {
                        for (int x = 0; x < big.width(); ++x) {
                            out.at(at.x * factor + x, at.y * factor + y) = big.at(x, y);
                        }
                    }
                };
                for (int code = 0; code < 256; ++code) { // 256 half-width glyphs
                    scaleCell(half(code), m_halfWidth);
                }
                for (int glyph = 0; glyph < BitmapFont::kFullGlyphCount; ++glyph) {
                    scaleCell(full(glyph), m_fullWidth);
                }
                return out;
            }

        private:
            /// @brief Gives how many full-width glyphs fit in one row.
            /// @return Glyphs per row.
            int perRow() const { return width() / m_fullWidth; }

            int m_halfWidth;
            int m_fullWidth;
            int m_glyphHeight;
    };

}

#endif
