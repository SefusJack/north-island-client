#ifndef NORTH_ISLAND_GFX_PICTURE_H
#define NORTH_ISLAND_GFX_PICTURE_H

#include "formats/Image.h"
#include "gfx/Texture.h"

#include <cstdint>
#include <vector>

namespace gfx {

    /// @brief A loaded picture: its texture plus which pixels are opaque (for pixel-exact hit tests).
    class Picture {
        public:
            Picture() = default;

            /// @brief Which pixels of a picture are opaque, made from its pixels on any thread before the upload.
            struct Mask {
                std::vector<std::uint64_t> bits; // one bit per pixel, row after row, 64 pixels per word
                core::Color topLeft{0, 0, 0, 0};

                /// @brief The mask of pixels at the logical size.
                /// @param image The pixels (an HD picture's shrunk to the logical size).
                /// @return The mask.
                static Mask of(const formats::Image& image);
            };

            /// @brief Makes a picture from its texture and pixels.
            /// @param texture The uploaded texture.
            /// @param image The pixels at the logical size (an HD picture's shrunk to it).
            Picture(
                Texture texture,
                const formats::Image& image
            );

            /// @brief Makes a picture from its texture and a mask made already (a prefetch worker's).
            /// @param texture The uploaded texture.
            /// @param mask The mask of its pixels at the logical size.
            Picture(
                Texture texture,
                Mask mask
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
            std::vector<std::uint64_t> m_opaque; // one bit per pixel, row after row, 64 pixels per word
            core::Color m_topLeft{0, 0, 0, 0};
    };

}

#endif
