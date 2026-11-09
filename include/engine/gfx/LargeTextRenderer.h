#ifndef NORTH_ISLAND_GFX_LARGE_TEXT_RENDERER_H
#define NORTH_ISLAND_GFX_LARGE_TEXT_RENDERER_H

#include "core/Point.h"
#include "formats/LargeBitmapFont.h"
#include "gfx/TextStyle.h"
#include "gfx/Texture.h"

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace gfx {

    class RenderDevice;

    /// @brief Draws text with the game's 24-pixel font; glyphs are made when first used.
    /// @details Original: DrawChinTextLarge. Half-width characters are 12 pixels apart (13 with an outline), full-width ones twice that.
    class LargeTextRenderer {
        public:
            /// @brief Creates the renderer over a font.
            /// @param font The large bitmap font.
            explicit LargeTextRenderer(
                formats::LargeBitmapFont font
            )
                : m_font(std::move(font)) {}

            /// @brief Draws text, wrapping to a new line when a character would pass maxWidth.
            /// @param device The render device.
            /// @param position Top left of the text.
            /// @param text The text in the game's encoding.
            /// @param style Colour and effect.
            /// @param maxWidth Width at which lines wrap.
            /// @param lineHeight Distance between lines.
            /// @return The y below the last line.
            int draw(
                const RenderDevice& device,
                core::Point position,
                const std::string& text,
                const TextStyle& style,
                int maxWidth,
                int lineHeight
            ) const;

            /// @brief Uses a pack's HD atlas for the glyphs from now on (formats::FontAtlasLayout::forLargeFont at twice or three times the size each way).
            /// @param atlas The atlas image.
            /// @return False (the font is kept) when the atlas is not such a size.
            bool useHdAtlas(const formats::Image& atlas);

        private:
            /// @brief Finds or makes the texture of a glyph.
            /// @param device The render device.
            /// @param full True for a full-width character.
            /// @param code The character code.
            /// @return The glyph texture, or nothing when the font has no such glyph.
            std::optional<std::reference_wrapper<const Texture>> glyphTexture(const RenderDevice& device, bool full, int code) const;

            /// @brief Draws one glyph with the style's effect.
            /// @param device The render device.
            /// @param glyph The glyph texture.
            /// @param at Top left of the character cell.
            /// @param cellWidth Width of the character cell.
            /// @param style Colour and effect.
            void drawGlyph(
                const RenderDevice& device,
                const Texture& glyph,
                core::Point at,
                int cellWidth,
                const TextStyle& style
            ) const;

            formats::LargeBitmapFont m_font;
            mutable std::map<std::pair<bool, int>, Texture> m_glyphs;
            std::vector<std::uint8_t> m_hdInk;
            int m_hdDetail = 1;
    };

}

#endif
