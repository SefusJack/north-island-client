#include "gfx/TextRenderer.h"

#include "core/Log.h"
#include "core/Speedups.h"
#include "gfx/RenderDevice.h"

#include <algorithm>
#include <iterator>

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
        // string pictures: they give the same pixels only when every glyph pixel is plain white or fully clear, so a
        // layer's colour lands as it is
        if (core::speedups().textPictures) {
            bool plain = true;
            for (const core::Color& pixel : atlas.pixels()) {
                plain = plain && (pixel.a == 0 || (pixel.a == 255 && pixel.r == 255 && pixel.g == 255 && pixel.b == 255));
            }
            if (plain) {
                renderer.m_atlasPixels = atlas;
            }
            core::logInfo("text: string pictures ", plain ? "on" : "off (the glyph atlas is not plain white)");
        }
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
        m_atlasPixels = formats::Image(); // an HD atlas is smoothed when drawn: no string pictures
        m_strings.clear();
        m_dropped.clear();
        return true;
    }

    std::optional<int> TextRenderer::drawCached(const RenderDevice& device, core::Point position, const std::string& key) const {
        // the clock is only read every so many strings: reading it costs as much as drawing one (before Windows 7 above all)
        constexpr int kStringsAClockRead = 64;
        constexpr std::uint64_t kSweepMs = 2000; // how often the strings seen but not kept are forgotten
        constexpr std::uint64_t kKeepMs = 5000; // a picture not drawn for this long is dropped
        if (m_clockAge++ % kStringsAClockRead == 0) {
            m_clockMs = SDL_GetTicks();
            // One picture may be made and one freed per clock read: Direct3D 12 and Vulkan take about 1 ms to make a
            // texture and 0.4 ms to free one (they wait for the graphics card), so many at once would hold a frame up.
            m_mayMake = true;
            if (!m_dropped.empty()) {
                m_dropped.pop_back();
            }
        }
        const std::uint64_t nowMs = m_clockMs;
        if (nowMs - m_sweptMs > kSweepMs) {
            m_sweptMs = nowMs;
            for (auto it = m_strings.begin(); it != m_strings.end();) {
                if (nowMs - it->second.usedMs > kKeepMs) {
                    m_dropped.push_back(std::move(it->second.texture)); // freed one at a time, above
                    it = m_strings.erase(it);
                } else {
                    ++it;
                }
            }
            for (auto it = m_seen.begin(); it != m_seen.end();) {
                it = nowMs - it->second > kSweepMs ? m_seen.erase(it) : std::next(it);
            }
        }
        if (const auto found = m_strings.find(key); found != m_strings.end()) {
            found->second.usedMs = nowMs;
            if (found->second.texture.isValid()) {
                found->second.texture.draw(device, {position.x + found->second.offset.x, position.y + found->second.offset.y});
            }
            return position.y + found->second.endY;
        }
        return std::nullopt;
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
        std::string key;
        // with a GPU renderer only at pixel scale 1: a scaled picture's edges between glyphs can be smoothed differently
        const bool pictures = batch != nullptr && !m_atlasPixels.isEmpty() && style.color.a == 255 && !text.empty() &&
                              (!device.isAccelerated() || device.pixelScale() == 1) &&
                              (!style.changing || device.makesTexturesQuickly()); // changing words: a picture each time only where that is cheap
        if (pictures) {
            key.reserve(text.size() + 16);
            key.append(text);
            key.push_back('\x1f');
            for (const int part : {static_cast<int>(style.color.r), static_cast<int>(style.color.g), static_cast<int>(style.color.b),
                                   static_cast<int>(style.effect), maxWidth, lineHeight}) {
                key.append(std::to_string(part));
                key.push_back(',');
            }
            if (const std::optional<int> end = drawCached(device, position, key)) {
                return *end;
            }
        }
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
            if (pictures && !m_batch.empty()) {
                const std::uint64_t nowMs = m_clockMs;
                const auto seen = m_seen.find(key);
                if (seen == m_seen.end()) {
                    m_seen.emplace(key, nowMs); // drawn the usual way this once: a string that changes every frame gets no picture
                } else if (m_mayMake) {
                    // seen before: the layers are put into one picture, last drawn on top, as the quads would be
                    m_mayMake = false;
                    int left = m_batch.front().destination.x;
                    int top = m_batch.front().destination.y;
                    int right = left;
                    int bottom = top;
                    for (const Texture::Quad& quad : m_batch) {
                        left = std::min(left, quad.destination.x);
                        top = std::min(top, quad.destination.y);
                        right = std::max(right, quad.destination.x + quad.source.width);
                        bottom = std::max(bottom, quad.destination.y + quad.source.height);
                    }
                    formats::Image picture(right - left, bottom - top);
                    for (const Texture::Quad& quad : m_batch) {
                        for (int row = 0; row < quad.source.height; ++row) {
                            const int fromY = quad.source.y + row;
                            if (fromY < 0 || fromY >= m_atlasPixels.height()) {
                                continue;
                            }
                            for (int column = 0; column < quad.source.width; ++column) {
                                const int fromX = quad.source.x + column;
                                if (fromX < 0 || fromX >= m_atlasPixels.width() || m_atlasPixels.at(fromX, fromY).a == 0) {
                                    continue;
                                }
                                picture.at(quad.destination.x - left + column, quad.destination.y - top + row) = {quad.color.r, quad.color.g, quad.color.b, 255};
                            }
                        }
                    }
                    CachedString made;
                    if (auto texture = Texture::create(device, picture)) {
                        made.texture = std::move(*texture);
                    }
                    made.offset = {left - position.x, top - position.y};
                    made.endY = y + lineHeight - position.y;
                    made.usedMs = nowMs;
                    m_seen.erase(seen);
                    const auto placed = m_strings.emplace(key, std::move(made)).first;
                    if (placed->second.texture.isValid()) {
                        placed->second.texture.draw(device, {left, top});
                        return y + lineHeight;
                    }
                }
            }
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
