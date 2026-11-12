#include "gfx/TextRenderer.h"

#include "gfx/RenderDevice.h"

#include <algorithm>

namespace gfx {

    namespace {

        constexpr int kHeight = formats::BitmapFont::kHeight;

        const core::Color kShadowColor = core::Color::fromRgb565(0x1082); // RGB565 0x1082: near-black shadow/outline colour
        constexpr std::uint16_t kColourKey = 0xF81F; // RGB565 0xF81F: magenta, the colour-key text colour
        const core::Color kColourKeyInk{240, 240, 240, 255}; // RGB: light grey ink drawn instead of colour-key magenta text

        /// @brief A pixel offset in x and y.
        struct Offset {
            int dx;
            int dy;
        };

        constexpr Offset kEdgeOffsets[] = {{1, 0}, {1, 1}, {0, 1}}; // px offsets of the coloured edge layers (right, below-right, below)
        constexpr Offset kOutlineShadowOffsets[] = {{2, 0}, {2, 1}, {2, 2}, {0, 2}, {1, 2}}; // px offsets of the dark shadow layers behind an outline
        constexpr Offset kShadowOffsets[] = {{1, 1}}; // px offset of the plain drop shadow (1 right, 1 down)

    }

    std::optional<TextRenderer> TextRenderer::create(const RenderDevice& device, const formats::BitmapFont& font) {
        const formats::FontAtlasLayout layout = formats::FontAtlasLayout::forSmallFont();
        const formats::Image atlas = layout.draw(font);
        auto texture = Texture::create(device, atlas);
        if (!texture) {
            return std::nullopt;
        }
        TextRenderer renderer;
        renderer.m_atlas = std::move(*texture);
        if (const int detail = device.pixelArtDetail(PixelArtPart::Text); detail > 1) {
            renderer.useHdAtlas(device, layout.scaledCells(atlas, detail, [&device, detail](const formats::Image& cell) {
                return formats::scalePixelArt(cell, device.pixelArtFilter(), detail);
            }));
        }
        return renderer;
    }

    bool TextRenderer::useHdAtlas(const RenderDevice& device, const formats::Image& atlas) {
        const formats::FontAtlasLayout layout = formats::FontAtlasLayout::forSmallFont();
        const int detail = atlas.width() / layout.width();
        // an HD atlas must be 2x or 3x the original size each way
        if (detail < 2 || detail > 3 || atlas.width() != layout.width() * detail || atlas.height() != layout.height() * detail) {
            return false;
        }
        auto texture = Texture::create(device, atlas, detail);
        if (!texture) {
            return false;
        }
        m_atlas = std::move(*texture);
        return true;
    }

    core::Color TextRenderer::edgeColorFor(core::Color text) {
        switch (text.toRgb565()) {
            // RGB565 text colour to its edge colour, as the original's SetTextColors pairs them
            case 0xFFFF: return core::Color::fromRgb565(0x8410); // white text: grey edge
            case 0xF800: return core::Color::fromRgb565(0xA800); // red text: dark red edge
            case 0x07E0: return core::Color::fromRgb565(0x0540); // green text: dark green edge
            case 0x001F: return core::Color::fromRgb565(0x0015); // blue text: dark blue edge
            case 0xF81F: return core::Color::fromRgb565(0xA815); // magenta (colour key) text: dark magenta edge
            case 0xFD55: return core::Color::fromRgb565(0xAD55); // light pink text: darker pink edge
            case 0xFFE0: return core::Color::fromRgb565(0xAD60); // yellow text: dark yellow edge
            case 0xFC00: return core::Color::fromRgb565(0xAAA0); // orange text: dark orange edge
            case 0x07FF: return core::Color::fromRgb565(0x0575); // cyan text: dark cyan edge
            case 0xC618: return core::Color::fromRgb565(0x528A); // light grey text: dark grey edge
            case 0x8410: return core::Color::fromRgb565(0x528A); // grey text: dark grey edge
            default: return text; // any other colour: no separate edge colour
        }
    }

    void TextRenderer::drawGlyph(
        const RenderDevice& device,
        const Glyph& glyph,
        core::Point at,
        int cellWidth,
        const TextStyle& style,
        std::vector<Texture::Quad>* batch
    ) const {
        const auto layer = [&](Offset offset, core::Color color) {
            const int width = std::min(glyph.width, cellWidth - offset.dx);
            const int height = kHeight - offset.dy;
            if (width <= 0 || height <= 0) {
                return;
            }
            if (batch) {
                batch->push_back({{at.x + offset.dx, at.y + offset.dy}, core::Rect{glyph.atlasX, glyph.atlasY, width, height}, color});
                return;
            }
            m_atlas.setColorMod(color);
            m_atlas.setAlphaMod(color.a);
            m_atlas.draw(device, {at.x + offset.dx, at.y + offset.dy}, core::Rect{glyph.atlasX, glyph.atlasY, width, height});
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
            const core::Color edge = edgeColorFor(style.color);
            for (Offset offset : kEdgeOffsets) {
                layer(offset, edge);
            }
        } else if (style.effect == TextEffect::Shadow) {
            for (Offset offset : kShadowOffsets) {
                layer(offset, kShadowColor);
            }
        }
        const bool keyed = style.effect == TextEffect::Plain && style.color.toRgb565() == kColourKey;
        layer({0, 0}, keyed ? kColourKeyInk : style.color);
    }

    int TextRenderer::draw(
        const RenderDevice& device,
        core::Point position,
        const std::string& text,
        const TextStyle& style,
        int maxWidth,
        int lineHeight
    ) const {
        const int step = charWidth(style.effect);
        const int extra = style.effect == TextEffect::Outline ? 1 : 0;
        int x = position.x;
        int y = position.y;
        // a background cell is filled before each glyph, between the glyphs' layers: those strings keep drawing glyph by glyph
        std::vector<Texture::Quad>* batch = style.background && style.effect != TextEffect::Shadow ? nullptr : &m_batch;
        m_batch.clear();
        for (std::size_t i = 0; i < text.size();) {
            const auto lead = static_cast<std::uint8_t>(text[i]);
            const std::optional<int> full = i + 1 < text.size() ? formats::BitmapFont::big5Glyph(lead, static_cast<std::uint8_t>(text[i + 1])) : std::nullopt;
            if (full) {
                if (x + step * 2 - position.x > maxWidth) {
                    x = position.x;
                    y += lineHeight;
                }
                const core::Point place = formats::FontAtlasLayout::forSmallFont().full(*full);
                const Glyph glyph{place.x, place.y, formats::BitmapFont::kFullWidth};
                drawGlyph(device, glyph, {x, y}, formats::BitmapFont::kFullWidth + 2 * extra, style, batch);
                x += step * 2;
                i += 2;
            } else {
                if (x + step - position.x > maxWidth) {
                    x = position.x;
                    y += lineHeight;
                }
                const Glyph glyph{formats::FontAtlasLayout::forSmallFont().half(lead).x, 0, formats::BitmapFont::kHalfWidth};
                drawGlyph(device, glyph, {x, y}, formats::BitmapFont::kHalfWidth + extra, style, batch);
                x += step;
                i += 1;
            }
        }
        if (batch) {
            m_atlas.drawQuads(device, m_batch);
            return y + lineHeight;
        }
        m_atlas.setColorMod({255, 255, 255, 255}); // back to the neutral colour modulation (opaque white)
        m_atlas.setAlphaMod(255); // back to fully opaque
        return y + lineHeight;
    }

    int TextRenderer::textWidth(const std::string& text, TextEffect effect) {
        const int step = charWidth(effect);
        int width = 0;
        for (std::size_t i = 0; i < text.size();) {
            const bool full = i + 1 < text.size() && formats::BitmapFont::big5Glyph(static_cast<std::uint8_t>(text[i]), static_cast<std::uint8_t>(text[i + 1]));
            width += full ? step * 2 : step;
            i += full ? 2 : 1;
        }
        return width;
    }

    std::vector<std::string> TextRenderer::wrapLines(const std::string& text, TextEffect effect, int maxWidth) {
        const int step = charWidth(effect);
        std::vector<std::string> lines;
        std::size_t start = 0; // first byte of the current line
        std::size_t space = std::string::npos; // the current line's last space
        int width = 0;
        int widthAfterSpace = 0;
        for (std::size_t i = 0; i < text.size();) {
            const bool full = i + 1 < text.size() && formats::BitmapFont::big5Glyph(static_cast<std::uint8_t>(text[i]), static_cast<std::uint8_t>(text[i + 1]));
            const int advance = full ? step * 2 : step;
            const bool isSpace = text[i] == ' ';
            if (width + advance > maxWidth && !isSpace) {
                if (space != std::string::npos) {
                    lines.push_back(text.substr(start, space - start));
                    start = space + 1;
                    width = widthAfterSpace;
                } else {
                    lines.push_back(text.substr(start, i - start));
                    start = i;
                    width = 0;
                }
                space = std::string::npos;
                widthAfterSpace = 0;
            }
            if (isSpace) {
                space = i;
                widthAfterSpace = 0;
            } else {
                widthAfterSpace += advance;
            }
            width += advance;
            i += full ? 2 : 1;
        }
        if (start < text.size()) {
            lines.push_back(text.substr(start));
        }
        return lines;
    }

}
