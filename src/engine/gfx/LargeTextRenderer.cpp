#include "gfx/LargeTextRenderer.h"

#include "formats/BitmapFont.h"
#include "formats/FontAtlas.h"
#include "gfx/RenderDevice.h"
#include "gfx/TextRenderer.h"

#include <algorithm>

namespace gfx {

    namespace {

        constexpr int kHeight = formats::LargeBitmapFont::kHeight;
        constexpr int kHalfShown = 12; // width of a half-width glyph cell actually drawn, px (the glyph bitmap is 16 wide)
        constexpr int kStep = 12; // advance per half-width character, px (a full-width one advances twice this)
        constexpr int kOutlineStep = 13; // advance per half-width character with the outline effect, px

        const core::Color kShadowColor = core::Color::fromRgb565(0x1082); // dark gray (RGB565 2, 4, 2) for shadows and outlines

        /// @brief A pixel offset in x and y.
        struct Offset {
            int dx;
            int dy;
        };

        // pixel offsets of the extra glyph copies, drawn before the glyph itself: outline edge in the edge colour, then the outline's outer shadow, or the plain shadow
        constexpr Offset kEdgeOffsets[] = {{1, 0}, {1, 1}, {0, 1}};
        constexpr Offset kOutlineShadowOffsets[] = {{2, 0}, {2, 1}, {2, 2}, {0, 2}, {1, 2}};
        constexpr Offset kShadowOffsets[] = {{1, 1}};

    }

    std::optional<std::reference_wrapper<const Texture>> LargeTextRenderer::glyphTexture(const RenderDevice& device, bool full, int code) const {
        const auto key = std::make_pair(full, code);
        if (const auto found = m_glyphs.find(key); found != m_glyphs.end()) {
            return std::cref(found->second);
        }
        const int width = full ? formats::LargeBitmapFont::kFullWidth : formats::LargeBitmapFont::kHalfWidth;
        const int detail = m_hdInk.empty() ? 1 : m_hdDetail;
        formats::Image image(width * detail, kHeight * detail);
        if (detail == 1) {
            const core::Color ink{255, 255, 255, 255}; // opaque white; the colour is applied when drawing
            for (int y = 0; y < kHeight; ++y) {
                for (int x = 0; x < width; ++x) {
                    if (full ? m_font.fullPixel(code, x, y) : m_font.halfPixel(static_cast<std::uint8_t>(code), x, y)) {
                        image.at(x, y) = ink;
                    }
                }
            }
        } else {
            const formats::FontAtlasLayout layout = formats::FontAtlasLayout::forLargeFont();
            const core::Point place = full ? layout.full(code) : layout.half(code);
            const int atlasWidth = layout.width() * detail;
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    const std::uint8_t alpha = m_hdInk
                        [static_cast<std::size_t>(place.y * detail + y) * static_cast<std::size_t>(atlasWidth) +
                         static_cast<std::size_t>(place.x * detail + x)];
                    image.at(x, y) = {255, 255, 255, alpha}; // white ink with the atlas's alpha
                }
            }
        }
        const int scaledDetail = detail == 1 ? device.pixelArtDetail(PixelArtPart::Text) : 1;
        auto texture = scaledDetail > 1 ? Texture::create(device, formats::scalePixelArt(image, device.pixelArtFilter(), scaledDetail), scaledDetail)
                                        : Texture::create(device, image, detail);
        if (!texture) {
            return std::nullopt;
        }
        return std::cref(m_glyphs.emplace(key, std::move(*texture)).first->second);
    }

    bool LargeTextRenderer::useHdAtlas(const formats::Image& atlas) {
        const formats::FontAtlasLayout layout = formats::FontAtlasLayout::forLargeFont();
        const int detail = atlas.width() / layout.width();
        // an HD atlas must be 2x or 3x the original size each way
        if (detail < 2 || detail > 3 || atlas.width() != layout.width() * detail || atlas.height() != layout.height() * detail) {
            return false;
        }
        m_hdDetail = detail;
        m_hdInk.clear();
        m_hdInk.reserve(atlas.pixels().size());
        for (const core::Color& pixel : atlas.pixels()) {
            m_hdInk.push_back(pixel.a);
        }
        m_glyphs.clear();
        return true;
    }

    void LargeTextRenderer::drawGlyph(
        const RenderDevice& device,
        const Texture& glyph,
        core::Point at,
        int cellWidth,
        const TextStyle& style
    ) const {
        const auto layer = [&](Offset offset, core::Color color) {
            const int width = std::min(glyph.width(), cellWidth - offset.dx);
            const int height = kHeight - offset.dy;
            if (width <= 0 || height <= 0) {
                return;
            }
            glyph.setColorMod(color);
            glyph.setAlphaMod(color.a);
            glyph.draw(device, {at.x + offset.dx, at.y + offset.dy}, core::Rect{0, 0, width, height});
        };

        if (style.background && style.effect != TextEffect::Shadow) {
            SDL_SetRenderDrawColor(device.renderer(), style.background->r, style.background->g, style.background->b, style.background->a);
            const SDL_FRect cell{static_cast<float>(at.x), static_cast<float>(at.y), static_cast<float>(cellWidth), static_cast<float>(kHeight)};
            SDL_RenderFillRect(device.renderer(), &cell);
        }
        if (style.effect == TextEffect::Outline) {
            for (Offset offset : kOutlineShadowOffsets) {
                layer(offset, kShadowColor);
            }
            const core::Color edge = TextRenderer::edgeColorFor(style.color);
            for (Offset offset : kEdgeOffsets) {
                layer(offset, edge);
            }
        } else if (style.effect == TextEffect::Shadow) {
            for (Offset offset : kShadowOffsets) {
                layer(offset, kShadowColor);
            }
        }
        layer({0, 0}, style.color);
        glyph.setColorMod({255, 255, 255, 255}); // back to the neutral colour modulation (opaque white)
        glyph.setAlphaMod(255); // back to fully opaque
    }

    int LargeTextRenderer::draw(
        const RenderDevice& device,
        core::Point position,
        const std::string& text,
        const TextStyle& style,
        int maxWidth,
        int lineHeight
    ) const {
        const bool outline = style.effect == TextEffect::Outline;
        const int step = outline ? kOutlineStep : kStep;
        int x = position.x;
        int y = position.y;
        for (std::size_t i = 0; i < text.size();) {
            const auto lead = static_cast<std::uint8_t>(text[i]);
            const std::optional<int> full = i + 1 < text.size() ? formats::BitmapFont::big5Glyph(lead, static_cast<std::uint8_t>(text[i + 1])) : std::nullopt;
            if (full) {
                if (x + step * 2 - position.x > maxWidth) {
                    x = position.x;
                    y += lineHeight;
                }
                if (const auto glyph = glyphTexture(device, true, *full)) {
                    drawGlyph(
                        device,
                        glyph->get(),
                        {x, y},
                        formats::LargeBitmapFont::kFullWidth + (outline ? 2 : 0),
                        style
                    ); // the outline widens the cell by 2 px
                }
                x += step * 2;
                i += 2;
            } else {
                if (x + step - position.x > maxWidth) {
                    x = position.x;
                    y += lineHeight;
                }
                if (const auto glyph = glyphTexture(device, false, lead)) {
                    drawGlyph(device, glyph->get(), {x, y}, kHalfShown + (outline ? 1 : 0), style); // the outline widens the cell by 1 px
                }
                x += step;
                i += 1;
            }
        }
        return y + lineHeight;
    }

}
