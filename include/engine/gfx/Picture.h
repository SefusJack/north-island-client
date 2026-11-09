#ifndef NORTH_ISLAND_GFX_PICTURE_H
#define NORTH_ISLAND_GFX_PICTURE_H

#include "formats/Image.h"
#include "gfx/Texture.h"

#include <vector>

namespace gfx {

    /// @brief A loaded picture: its texture plus which pixels are opaque (for pixel-exact hit tests).
    class Picture {
        public:
            Picture() = default;

            /// @brief Makes a picture from its texture and pixels.
            /// @param texture The uploaded texture.
            /// @param image The pixels at the logical size (an HD picture's shrunk to it).
            Picture(
                Texture texture,
                const formats::Image& image
            );

            /// @brief The picture's texture.
            /// @return The texture.
            const Texture& texture() const { return m_texture; }

            /// @brief The width in logical pixels.
            /// @return The width.
            int width() const { return m_texture.width(); }

            /// @brief The height in logical pixels.
            /// @return The height.
            int height() const { return m_texture.height(); }

            /// @brief Tells whether a pixel is opaque.
            /// @param x Pixel column.
            /// @param y Pixel row.
            /// @return True when the pixel is opaque.
            bool isOpaque(int x, int y) const;

            /// @brief The top-left pixel (some ground objects mark themselves with it).
            /// @return The colour of that pixel.
            core::Color topLeft() const { return m_topLeft; }

        private:
            Texture m_texture;
            std::vector<bool> m_opaque;
            core::Color m_topLeft{0, 0, 0, 0};
    };

}

#endif
