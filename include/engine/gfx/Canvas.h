#ifndef NORTH_ISLAND_GFX_CANVAS_H
#define NORTH_ISLAND_GFX_CANVAS_H

#include "core/Color.h"
#include "core/Point.h"
#include "core/Rect.h"
#include "gfx/PictureId.h"
#include "gfx/TextStyle.h"

#include "gfx/Texture.h"

#include <cstdint>
#include <optional>
#include <string>

namespace gfx {

    class PictureLibrary;
    class Texture;
    class RenderDevice;
    class TextRenderer;
    class LargeTextRenderer;

    /// @brief The drawing surface screens and controls use: pictures by id, filled rectangles, outlines and text, in the game's 800 x 600 coordinates.
    class Canvas {
        public:
            /// @brief Creates a canvas over a render device and the picture and text sources.
            /// @param device The render device drawn on.
            /// @param pictures The picture library pictures are looked up in.
            /// @param text The small text renderer.
            /// @param largeText The large text renderer.
            Canvas(
                const RenderDevice& device,
                PictureLibrary& pictures,
                const TextRenderer& text,
                const LargeTextRenderer& largeText
            )
                : m_device(device),
                  m_pictures(pictures),
                  m_text(text),
                  m_largeText(largeText) {}

            /// @brief The picture library of this canvas.
            /// @return The library.
            PictureLibrary& pictures() { return m_pictures; }

            /// @brief Draws a picture (or a part of it).
            /// @param picture The picture id.
            /// @param at Top left position in canvas coordinates.
            /// @param source Part of the picture to draw; the whole picture when empty.
            void drawPicture(PictureId picture, core::Point at, const std::optional<core::Rect>& source = std::nullopt);

            /// @brief Draws a picture (or a part of it) turned over.
            /// @param picture The picture id.
            /// @param at Top left position in canvas coordinates.
            /// @param leftRight Turn over left to right.
            /// @param topBottom Turn over top to bottom.
            /// @param source Part of the picture to draw; the whole picture when empty.
            void drawPictureFlipped(
                PictureId picture,
                core::Point at,
                bool leftRight,
                bool topBottom,
                const std::optional<core::Rect>& source = std::nullopt
            );

            /// @brief Draws a picture stretched or shrunk into a rectangle.
            /// @details Original: StretchDrawPic.
            /// @param picture The picture id.
            /// @param destination The rectangle to fill.
            /// @param source Part of the picture to draw; the whole picture when empty.
            void drawPictureScaled(PictureId picture, const core::Rect& destination, const std::optional<core::Rect>& source = std::nullopt);

            /// @brief Draws a picture additively as light.
            /// @details Original: rodraw2's LightAlpha blit; adds strength / 32 of the picture's colours to the screen, each channel saturating.
            /// @param picture The picture id.
            /// @param at Top left position in canvas coordinates.
            /// @param strength Light strength (x / 32).
            /// @param source Part of the picture to draw; the whole picture when empty.
            void drawPictureLight(PictureId picture, core::Point at, int strength, const std::optional<core::Rect>& source = std::nullopt);

            /// @brief Draws a texture the caller owns (sprites built at run time).
            /// @param texture The texture.
            /// @param at Top left position in canvas coordinates.
            /// @param source Part of the texture to draw; the whole texture when empty.
            void drawTexture(const Texture& texture, core::Point at, const std::optional<core::Rect>& source = std::nullopt);

            /// @brief Draws the whole texture turned over left to right.
            /// @param texture The texture.
            /// @param at Top left position in canvas coordinates.
            void drawTextureMirrored(const Texture& texture, core::Point at);

            /// @brief Adds a share of the texture's colours to the screen.
            /// @details Original: rodraw2's additive blit; adds level / 255 of the colours.
            /// @param texture The texture.
            /// @param at Top left position in canvas coordinates.
            /// @param level Strength, 0..255.
            /// @param source Part of the texture to draw; the whole texture when empty.
            void addTexture(const Texture& texture, core::Point at, std::uint8_t level, const std::optional<core::Rect>& source = std::nullopt);

            /// @brief Draws a texture translucent over what lies below.
            /// @details Original: rodraw2's alpha blit; alpha / 255 of the texture.
            /// @param texture The texture.
            /// @param at Top left position in canvas coordinates.
            /// @param alpha Opacity, 0..255.
            /// @param source Part of the texture to draw; the whole texture when empty.
            void blendTexture(const Texture& texture, core::Point at, std::uint8_t alpha, const std::optional<core::Rect>& source = std::nullopt);

            /// @brief Draws a texture translucent and stretched into a rectangle.
            /// @param texture The texture.
            /// @param destination The rectangle to fill.
            /// @param alpha Opacity, 0..255.
            void blendTextureScaled(const Texture& texture, const core::Rect& destination, std::uint8_t alpha);

            /// @brief Draws a texture stretched into a rectangle.
            /// @param texture The texture.
            /// @param destination The rectangle to fill.
            /// @param addLevel Additive level; 0 draws it plainly.
            void drawTextureScaled(const Texture& texture, const core::Rect& destination, std::uint8_t addLevel = 0);

            /// @brief The render device of this canvas.
            /// @return The device.
            const RenderDevice& device() const { return m_device; }

            /// @brief Fills a rectangle with a colour.
            /// @param rect The rectangle.
            /// @param color The fill colour.
            void fillRect(const core::Rect& rect, core::Color color);

            /// @brief Fills a rectangle with a colour whose alpha is the strength, blended over the screen.
            /// @details Original: TDirectDrawSurface.FillRectAlpha, blended as the 16-bit screen did (formats::fillAlpha16); an opaque colour is a plain fill.
            /// @param rect The rectangle.
            /// @param color The colour; its alpha is the strength.
            void fillRectAlpha(const core::Rect& rect, core::Color color);

            /// @brief Chooses exact 16-bit alpha fills or the renderer's own blend.
            /// @param exact True for the exact blend.
            void setExactAlphaFills(bool exact) { m_exactAlphaFills = exact; }

            /// @brief Whether alpha fills are blended exactly.
            /// @return True for the exact blend.
            bool exactAlphaFills() const { return m_exactAlphaFills; }

            /// @brief Copies what has been drawn on the window so far into a texture.
            /// @return The texture, or nothing when it cannot be read.
            std::optional<Texture> copyWindow() const;

            /// @brief Draws the outline of a rectangle.
            /// @param rect The rectangle.
            /// @param color The line colour.
            /// @param thickness Line width in pixels.
            void drawRectOutline(const core::Rect& rect, core::Color color, int thickness = 1);

            /// @brief Draws a line with a wide round-ended pen (a GDI pen of that width, core::thickLineRows).
            /// @param from Start point.
            /// @param to End point.
            /// @param width Pen width in pixels.
            /// @param color Line colour.
            void drawLine(core::Point from, core::Point to, int width, core::Color color);

            /// @brief Draws text with the small text renderer (see TextRenderer::draw).
            /// @param at Position of the text.
            /// @param text The text.
            /// @param style Colour and style.
            /// @param maxWidth Width at which lines wrap.
            /// @param lineHeight Height of a line.
            /// @return What TextRenderer::draw returns.
            int drawText(
                core::Point at,
                const std::string& text,
                const TextStyle& style,
                int maxWidth = 100000,
                int lineHeight = 15
            ); // defaults: maxWidth 100000 = no wrapping, lineHeight 15 = the small font's height

            /// @brief Draws text with the large text renderer (see LargeTextRenderer::draw).
            /// @details Original: DrawChinTextLarge.
            /// @param at Position of the text.
            /// @param text The text.
            /// @param style Colour and style.
            /// @param maxWidth Width at which lines wrap.
            /// @param lineHeight Height of a line.
            /// @return What LargeTextRenderer::draw returns.
            int drawLargeText(
                core::Point at,
                const std::string& text,
                const TextStyle& style,
                int maxWidth,
                int lineHeight
            );

            /// @brief Restricts drawing to a rectangle.
            /// @param clip The clip rectangle, or nothing to remove the clip.
            void setClip(const std::optional<core::Rect>& clip);

            /// @brief Starts drawing everything scale times as big around a fixed pivot (the HUD size).
            /// @param pivot Canvas point that stays where it is.
            /// @param scale Scale factor.
            void beginScale(core::Point pivot, float scale);

            /// @brief Goes back to scale 1.
            void endScale();

            /// @brief The current scale.
            /// @return The scale factor.
            float scale() const { return m_scale; }

            /// @brief Sets where the game screen's top left lies in the window.
            /// @param origin The origin.
            void setOrigin(core::Point origin) { m_origin = origin; }

            /// @brief Where the game screen's top left lies in the window.
            /// @return The origin.
            core::Point origin() const { return m_origin; }

            /// @brief The whole window in the canvas's coordinates.
            /// @return The window rectangle.
            core::Rect window() const;

            /// @brief Starts drawing into a black-cleared buffer that covers an area; a part of it is later stretched over the screen.
            /// @param area The area in canvas coordinates (window() is it meanwhile).
            void beginZoom(const core::Rect& area);

            /// @brief Stretches a part of the zoom buffer over the whole screen.
            /// @details Original: StretchBufToScreen.
            /// @param source The part, in canvas coordinates, inside the area.
            void endZoom(const core::Rect& source);

            /// @brief Whether a zoom is in progress.
            /// @return True between beginZoom and endZoom.
            bool isZooming() const { return m_zooming; }

            /// @brief Sets whether this frame steps what the game steps once per drawn frame.
            /// @param on True when this frame is such a step frame.
            void setAnimationFrame(bool on) { m_animationFrame = on; }

            /// @brief Whether this frame steps what the game steps once per drawn frame.
            /// @return True for a step frame.
            bool isAnimationFrame() const { return m_animationFrame; }

            /// @brief Sets how far this frame is between two beats (free frames only).
            /// @param amount 0 on the beat, nearly 1 just before the next; nothing at the 30 ms beat.
            /// @param beatMs Length of the beat in milliseconds.
            void setBetweenBeats(std::optional<double> amount, std::uint64_t beatMs) {
                m_betweenBeats = amount;
                m_beatMs = beatMs;
            }

            /// @brief How far this frame is between two beats.
            /// @return The amount, or nothing at the 30 ms beat.
            std::optional<double> betweenBeats() const { return m_betweenBeats; }

            /// @brief The beat length.
            /// @return Milliseconds of a beat.
            std::uint64_t beatMs() const { return m_beatMs; }

        private:
            const RenderDevice& m_device;
            PictureLibrary& m_pictures;
            const TextRenderer& m_text;
            const LargeTextRenderer& m_largeText;
            bool m_zooming = false;
            core::Rect m_zoomArea;
            core::Point m_originBeforeZoom;
            bool m_exactAlphaFills = true;
            core::Point m_origin;
            float m_scale = 1.0f;
            core::Point m_originBeforeScale;
            bool m_animationFrame = true;
            std::optional<double> m_betweenBeats;
            std::uint64_t m_beatMs = 0;
    };

}

#endif
