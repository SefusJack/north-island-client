#include "gfx/Canvas.h"

#include "core/ThickLine.h"
#include "formats/GamePixels.h"
#include "gfx/PictureLibrary.h"
#include "gfx/RenderDevice.h"
#include "gfx/LargeTextRenderer.h"
#include "gfx/TextRenderer.h"

#include <algorithm>
#include <cmath>

namespace gfx {

    namespace {

        /// @brief Moves a rectangle.
        /// @param rect The rectangle.
        /// @param by The offset to add to its position.
        /// @return The moved rectangle.
        core::Rect moved(const core::Rect& rect, core::Point by) { return {rect.x + by.x, rect.y + by.y, rect.width, rect.height}; }

    }

    void Canvas::drawPicture(PictureId picture, core::Point at, const std::optional<core::Rect>& source) {
        if (!picture.isValid()) {
            return;
        }
        m_pictures.picture(picture).texture().draw(m_device, at + m_origin, source);
    }

    void Canvas::drawPictureFlipped(
        PictureId picture,
        core::Point at,
        bool leftRight,
        bool topBottom,
        const std::optional<core::Rect>& source
    ) {
        if (picture.isValid()) {
            m_pictures.picture(picture).texture().drawFlipped(m_device, at + m_origin, leftRight, topBottom, source);
        }
    }

    void Canvas::drawPictureScaled(PictureId picture, const core::Rect& destination, const std::optional<core::Rect>& source) {
        if (picture.isValid()) {
            m_pictures.picture(picture).texture().drawScaled(m_device, moved(destination, m_origin), source);
        }
    }

    void Canvas::drawPictureLight(PictureId picture, core::Point at, int strength, const std::optional<core::Rect>& source) {
        constexpr int kFull = 32; // light strength of one full-brightness additive pass
        constexpr int kMostPasses = 8; // most additive passes drawn (strength is capped at kFull * kMostPasses = 256)
        if (!picture.isValid() || strength <= 0) {
            return;
        }
        const Texture& texture = m_pictures.picture(picture).texture();
        texture.setAdditive(true);
        for (int left = std::min(strength, kFull * kMostPasses); left > 0; left -= kFull) {
            const auto level = static_cast<std::uint8_t>(std::min(left, kFull) * 255 / kFull); // 255: full channel value; level is the pass's share of it
            texture.setColorMod({level, level, level, 255});
            texture.draw(m_device, at + m_origin, source);
        }
        texture.setColorMod({255, 255, 255, 255}); // back to the neutral colour modulation (opaque white)
        texture.setAdditive(false);
    }

    void Canvas::drawTexture(const Texture& texture, core::Point at, const std::optional<core::Rect>& source) { texture.draw(m_device, at + m_origin, source); }

    void Canvas::drawTextureMirrored(const Texture& texture, core::Point at) { texture.drawMirrored(m_device, at + m_origin); }

    void Canvas::addTexture(const Texture& texture, core::Point at, std::uint8_t level, const std::optional<core::Rect>& source) {
        texture.setAdditive(true);
        texture.setColorMod({level, level, level, 255});
        texture.draw(m_device, at + m_origin, source);
        texture.setColorMod({255, 255, 255, 255}); // back to the neutral colour modulation (opaque white)
        texture.setAdditive(false);
    }

    void Canvas::blendTexture(const Texture& texture, core::Point at, std::uint8_t alpha, const std::optional<core::Rect>& source) {
        texture.setAlphaMod(alpha);
        texture.draw(m_device, at + m_origin, source);
        texture.setAlphaMod(255); // back to fully opaque
    }

    void Canvas::blendTextureScaled(const Texture& texture, const core::Rect& destination, std::uint8_t alpha) {
        texture.setAlphaMod(alpha);
        texture.drawScaled(m_device, moved(destination, m_origin), std::nullopt);
        texture.setAlphaMod(255); // back to fully opaque
    }

    void Canvas::drawTextureScaled(const Texture& texture, const core::Rect& destination, std::uint8_t addLevel) {
        if (addLevel == 0) {
            texture.drawScaled(m_device, moved(destination, m_origin), std::nullopt);
            return;
        }
        texture.setAdditive(true);
        texture.setColorMod({addLevel, addLevel, addLevel, 255});
        texture.drawScaled(m_device, moved(destination, m_origin), std::nullopt);
        texture.setColorMod({255, 255, 255, 255}); // back to the neutral colour modulation (opaque white)
        texture.setAdditive(false);
    }

    void Canvas::fillRect(const core::Rect& rect, core::Color color) {
        const core::Color shown = core::Color::fromRgb565(color.toRgb565());
        SDL_SetRenderDrawColor(m_device.renderer(), shown.r, shown.g, shown.b, color.a);
        const SDL_FRect area{
            static_cast<float>(rect.x + m_origin.x),
            static_cast<float>(rect.y + m_origin.y),
            static_cast<float>(rect.width),
            static_cast<float>(rect.height)
        };
        SDL_RenderFillRect(m_device.renderer(), &area);
    }

    void Canvas::fillRectAlpha(const core::Rect& rect, core::Color color) {
        if (color.a == 255) { // opaque
            fillRect(rect, color);
            return;
        }
        if (color.a == 0) {
            return;
        }
        if (!m_exactAlphaFills || m_scale != 1.0f) {
            fillRect(rect, color);
            return;
        }
        const core::Rect onWindow = moved(rect, m_origin);
        formats::Image area = m_device.readRegion(onWindow);
        if (area.isEmpty()) {
            return;
        }
        formats::fillAlpha16(area, color);
        if (const auto texture = Texture::create(m_device, area)) {
            texture->draw(m_device, {std::max(onWindow.x, 0), std::max(onWindow.y, 0)});
        }
    }

    std::optional<Texture> Canvas::copyWindow() const {
        const auto [frame, detail] = m_device.readFrameDetailed();
        return Texture::create(m_device, frame, detail);
    }

    void Canvas::drawRectOutline(const core::Rect& rect, core::Color color, int thickness) {
        for (int i = 0; i < thickness; ++i) {
            const core::Rect edge{rect.x + i, rect.y + i, rect.width - 2 * i, rect.height - 2 * i};
            fillRect({edge.x, edge.y, edge.width, 1}, color);
            fillRect({edge.x, edge.bottom() - 1, edge.width, 1}, color);
            fillRect({edge.x, edge.y, 1, edge.height}, color);
            fillRect({edge.right() - 1, edge.y, 1, edge.height}, color);
        }
    }

    void Canvas::drawLine(core::Point from, core::Point to, int width, core::Color color) {
        for (const core::Rect& row : core::thickLineRows(from, to, width)) {
            fillRect(row, color);
        }
    }

    int Canvas::drawText(
        core::Point at,
        const std::string& text,
        const TextStyle& style,
        int maxWidth,
        int lineHeight
    ) {
        return m_text.draw(m_device, at + m_origin, text, style, maxWidth, lineHeight);
    }

    int Canvas::drawLargeText(
        core::Point at,
        const std::string& text,
        const TextStyle& style,
        int maxWidth,
        int lineHeight
    ) {
        return m_largeText.draw(m_device, at + m_origin, text, style, maxWidth, lineHeight);
    }

    void Canvas::beginScale(core::Point pivot, float scale) {
        if (m_scale != 1.0f) {
            endScale();
        }
        if (scale == 1.0f) {
            return;
        }
        m_scale = scale;
        m_originBeforeScale = m_origin;
        m_origin = {
            static_cast<int>(std::lround((pivot.x + m_origin.x) / scale - pivot.x)),
            static_cast<int>(std::lround((pivot.y + m_origin.y) / scale - pivot.y))
        };
        SDL_SetRenderScale(m_device.renderer(), scale, scale);
    }

    void Canvas::endScale() {
        if (m_scale == 1.0f) {
            return;
        }
        SDL_SetRenderScale(m_device.renderer(), 1.0f, 1.0f);
        m_origin = m_originBeforeScale;
        m_scale = 1.0f;
    }

    void Canvas::setClip(const std::optional<core::Rect>& clip) { m_device.setClip(clip ? std::optional<core::Rect>(moved(*clip, m_origin)) : std::nullopt); }

    core::Rect Canvas::window() const { return m_zooming ? m_zoomArea : core::Rect{-m_origin.x, -m_origin.y, m_device.width(), m_device.height()}; }

    void Canvas::beginZoom(const core::Rect& area) {
        m_device.beginZoomedFrame({area.width, area.height});
        m_zooming = true;
        m_zoomArea = area;
        m_originBeforeZoom = m_origin;
        m_origin = {-area.x, -area.y};
    }

    void Canvas::endZoom(const core::Rect& source) {
        if (m_zooming) {
            m_device.endZoomedFrame(moved(source, m_origin));
            m_zooming = false;
            m_origin = m_originBeforeZoom;
        }
    }

}
