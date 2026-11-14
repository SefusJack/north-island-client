#ifndef NORTH_ISLAND_GFX_TEXTURE_H
#define NORTH_ISLAND_GFX_TEXTURE_H

#include "core/Color.h"
#include "core/Point.h"
#include "core/Rect.h"
#include "formats/Image.h"
#include "gfx/SdlDeleter.h"

#include <optional>
#include <vector>

namespace gfx {

    class RenderDevice;

    /// @brief An image uploaded to the renderer (video memory with an accelerated renderer).
    /// @details Its size and every rectangle given to it are in logical pixels; an HD texture has detail x detail of its own
    /// pixels to each and is smoothed when drawn.
    class Texture {
        public:
            Texture() = default;

            /// @brief Uploads an image.
            /// @param device The render device.
            /// @param image The pixels.
            /// @param detail How many of the image's pixels across make one logical pixel (a mod pack's @2x picture: 2).
            /// @return The texture, or nothing when the upload fails.
            static std::optional<Texture> create(const RenderDevice& device, const formats::Image& image, int detail = 1);

            /// @brief Makes an opaque texture whose pixels are given later, some rows at a time (a map background decoded in parts).
            /// @param device The render device.
            /// @param width Width in pixels.
            /// @param height Height in pixels.
            /// @return The texture, or nothing when it cannot be made.
            static std::optional<Texture> createOpaque(const RenderDevice& device, int width, int height);

            /// @brief Gives some rows of a createOpaque texture their pixels.
            /// @param top The first row.
            /// @param rows How many rows.
            /// @param pixels width() pixels a row, all opaque; they may be changed (turned into the texture's own format).
            void setRows(int top, int rows, core::Color* pixels) const;

            /// @brief The width in logical pixels.
            /// @return The width.
            int width() const { return m_width; }

            /// @brief The height in logical pixels.
            /// @return The height.
            int height() const { return m_height; }

            /// @brief How many of the texture's pixels across make one logical pixel.
            /// @return The detail.
            int detail() const { return m_detail; }

            /// @brief Tells whether the texture holds an uploaded image.
            /// @return True when valid.
            bool isValid() const { return static_cast<bool>(m_texture); }

            /// @brief Draws source (or everything) at destination, unscaled.
            /// @param device The render device.
            /// @param destination Top left position.
            /// @param source Part to draw; everything when empty.
            void draw(const RenderDevice& device, core::Point destination, const std::optional<core::Rect>& source = std::nullopt) const;

            /// @brief One part of the texture to draw unscaled, tinted as setColorMod and setAlphaMod would.
            struct Quad {
                core::Point destination;
                core::Rect source;
                core::Color color{255, 255, 255, 255};
            };

            /// @brief Draws many parts in one SDL call, in order; the same pixels as draw with each quad's colour and alpha mods.
            /// @details SDL_RenderTexture becomes the same geometry inside SDL, with the mods as the vertex colour.
            /// @param device The render device.
            /// @param quads The parts, first drawn first.
            void drawQuads(const RenderDevice& device, const std::vector<Quad>& quads) const;

            /// @brief Draws everything at destination, turned over left to right.
            /// @param device The render device.
            /// @param destination Top left position.
            void drawMirrored(const RenderDevice& device, core::Point destination) const { drawFlipped(device, destination, true, false); }

            /// @brief Draws source (or everything) at destination, turned over left to right and / or top to bottom.
            /// @param device The render device.
            /// @param destination Top left position.
            /// @param leftRight Turn over left to right.
            /// @param topBottom Turn over top to bottom.
            /// @param source Part to draw; everything when empty.
            void drawFlipped(
                const RenderDevice& device,
                core::Point destination,
                bool leftRight,
                bool topBottom,
                const std::optional<core::Rect>& source = std::nullopt
            ) const;

            /// @brief Draws source scaled into destination.
            /// @param device The render device.
            /// @param destination The rectangle to fill.
            /// @param source Part to draw; everything when empty.
            void drawScaled(const RenderDevice& device, const core::Rect& destination, const std::optional<core::Rect>& source = std::nullopt) const;

            /// @brief Sets the colour the texture is multiplied by when drawn.
            /// @param color The colour modulation.
            void setColorMod(core::Color color) const;

            /// @brief Sets the opacity the texture is drawn with.
            /// @param alpha Opacity, 0..255.
            void setAlphaMod(std::uint8_t alpha) const;

            /// @brief Makes the texture add its colours to what lies below instead of covering it.
            /// @param additive True for additive drawing.
            void setAdditive(bool additive) const;

        private:
            /// @brief The texture for draws that are scaled, flipped, tinted, faded, additive or geometry.
            /// @details With the software renderer the main texture is static, which SDL run-length encodes: opaque runs are
            /// stored in the screen's own format and copied, transparent runs skipped. SDL has to decode and encode it again
            /// each time a colour, alpha or blend change comes and goes, so those draws use a second, plain copy, made from the
            /// main one the first time it is needed. With a GPU renderer this is the main texture.
            /// @param device The render device.
            /// @return The texture, with the colour, alpha and blend settings applied.
            SDL_Texture* loose(const RenderDevice& device) const;

            SdlHandle<SDL_Texture> m_texture;
            mutable SdlHandle<SDL_Texture> m_loose; // software renderer only: the plain copy
            mutable bool m_looseFailed = false;
            mutable bool m_looseStale = true; // the settings below are not on m_loose yet
            bool m_encoded = false; // software renderer: m_texture is static (encoded), settings go to m_loose
            bool m_opaque = false; // made by createOpaque: drawn without blending while it is not faded or additive
            bool m_swapRedBlue = false; // made by createOpaque in the software screen's own format (blue first)

            /// @brief Puts the blend mode for the current settings on the main texture.
            void applyBlend() const;
            mutable core::Color m_colorMod{255, 255, 255, 255};
            mutable std::uint8_t m_alphaMod = 255;
            mutable bool m_additive = false;
            int m_width = 0;
            int m_height = 0;
            int m_detail = 1;
    };

}

#endif
