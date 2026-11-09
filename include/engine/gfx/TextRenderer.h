#ifndef NORTH_ISLAND_GFX_TEXT_RENDERER_H
#define NORTH_ISLAND_GFX_TEXT_RENDERER_H

#include "core/Point.h"
#include "formats/BitmapFont.h"
#include "formats/FontAtlas.h"
#include "gfx/TextStyle.h"
#include "gfx/Texture.h"

#include <optional>
#include <string>
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
            void drawGlyph(
                const RenderDevice& device,
                const Glyph& glyph,
                core::Point at,
                int cellWidth,
                const TextStyle& style
            ) const;

            Texture m_atlas;
    };

}

#endif
