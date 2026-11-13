#ifndef NORTH_ISLAND_GFX_TEXT_RENDERER_H
#define NORTH_ISLAND_GFX_TEXT_RENDERER_H

#include "core/Point.h"
#include "formats/BitmapFont.h"
#include "formats/FontAtlas.h"
#include "gfx/TextStyle.h"
#include "gfx/Texture.h"

#include <optional>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace gfx {

    class RenderDevice;

    /// @brief Draws text with the game's 15-pixel bitmap font (Big5: ASCII bytes are half-width, Big5 pairs full-width).
    /// @details All glyphs live in one texture; colours and effects are applied when drawing.
    class TextRenderer {
        public:
            static constexpr int kLineHeight = formats::BitmapFont::kHeight;

            /// @brief Builds the glyph atlas texture from the font.
            /// @param device The render device.
            /// @param font The bitmap font.
            /// @return The renderer, or nothing when the atlas cannot be made.
            static std::optional<TextRenderer> create(const RenderDevice& device, const formats::BitmapFont& font);

            /// @brief Uses a pack's HD atlas (formats::FontAtlasLayout::forSmallFont at twice or three times the size each way) instead of the font's.
            /// @param device The render device.
            /// @param atlas The atlas image.
            /// @return False (the font's is kept) when the atlas is not such a size or cannot be uploaded.
            bool useHdAtlas(const RenderDevice& device, const formats::Image& atlas);

            /// @brief Draws text, wrapping to a new line when a character would pass maxWidth.
            /// @param device The render device.
            /// @param position Top left of the text.
            /// @param text The text in the game's encoding.
            /// @param style Colour and effect.
            /// @param maxWidth Width at which lines wrap.
            /// @param lineHeight Distance between lines.
            /// @return The y below the last line (y + lineHeight per line started, as the original returned).
            int draw(
                const RenderDevice& device,
                core::Point position,
                const std::string& text,
                const TextStyle& style,
                int maxWidth = 100000,
                int lineHeight = kLineHeight
            ) const; // default maxWidth: effectively no wrapping

            /// @brief The advance of one half-width character for a style.
            /// @param effect The text effect.
            /// @return 8 pixels, or 9 with an outline.
            static int charWidth(TextEffect effect) { return effect == TextEffect::Outline ? 9 : 8; } // half-width advance: 9 outlined, 8 otherwise

            /// @brief The width of a text in pixels.
            /// @param text The text in the game's encoding.
            /// @param effect The text effect.
            /// @return The width.
            static int textWidth(const std::string& text, TextEffect effect);

            /// @brief Splits a text into lines that fit a width, breaking at spaces; a word wider than the width is broken where it runs out.
            /// @param text The text in the game's encoding.
            /// @param effect The text effect.
            /// @param maxWidth The widest line, in pixels.
            /// @return The lines, without the spaces they were broken at.
            static std::vector<std::string> wrapLines(const std::string& text, TextEffect effect, int maxWidth);

            /// @brief The edge colour the game gives a text colour.
            /// @details Original: SetTextColors.
            /// @param text The text colour.
            /// @return The edge colour.
            static core::Color edgeColorFor(core::Color text);

        private:
            /// @brief Where a glyph lies in the atlas.
            struct Glyph {
                int atlasX = 0;
                int atlasY = 0;
                int width = 0;
            };

            /// @brief Draws one glyph with the style's effect.
            /// @param device The render device.
            /// @param glyph The glyph's atlas place.
            /// @param at Top left of the character cell.
            /// @param cellWidth Width of the character cell.
            /// @param style Colour and effect.
            /// @param batch Where to add the glyph's layers, to draw them in one call later; null draws them now.
            void drawGlyph(
                const RenderDevice& device,
                const Glyph& glyph,
                core::Point at,
                int cellWidth,
                const TextStyle& style,
                std::vector<Texture::Quad>* batch
            ) const;

            /// @brief A string drawn once into its own picture (software renderer): one copy instead of up to nine a glyph.
            struct CachedString {
                Texture texture;
                core::Point offset; // of the picture from the string's position
                int endY = 0; // what draw returns, from the string's position
                std::uint64_t usedMs = 0;
            };

            /// @brief Draws the string from its kept picture; every so many calls it reads the clock and frees one picture no longer drawn.
            /// @return The y below the text, or nothing when the string is drawn the usual way.
            std::optional<int> drawCached(const RenderDevice& device, core::Point position, const std::string& key) const;

            Texture m_atlas;
            mutable std::vector<Texture::Quad> m_batch; // the layers of the string being drawn, reused between strings
            formats::Image m_atlasPixels; // kept for the string pictures (empty when they are not used)
            mutable std::unordered_map<std::string, CachedString> m_strings;
            mutable std::unordered_map<std::string, std::uint64_t> m_seen; // strings drawn once so far, and when
            mutable std::uint64_t m_sweptMs = 0;
            mutable std::uint64_t m_clockMs = 0; // the time of the last clock read, used for the string pictures' ages
            mutable unsigned m_clockAge = 0; // strings since that read
            mutable bool m_mayMake = false; // no picture has been made since the last clock read
            mutable std::vector<Texture> m_dropped; // pictures no longer drawn, freed one per clock read
    };

}

#endif
