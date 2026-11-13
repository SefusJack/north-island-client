#ifndef NORTH_ISLAND_GFX_PICTURE_H
#define NORTH_ISLAND_GFX_PICTURE_H

#include "formats/Image.h"
#include "gfx/Texture.h"

#include <cstdint>
#include <vector>

namespace gfx {

    /// @brief A loaded picture: its texture plus which pixels are opaque (for pixel-exact hit tests).
    class PictureLibrary;

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
            const Texture& texture() const {
                load();
                return m_texture;
            }

            /// @brief The picture's texture, for a draw that shows only some of its rows.
            /// @details A picture decoded in parts gets those rows now and keeps the rest for later; any other picture is loaded as texture() does.
            /// @param top The first row drawn.
            /// @param bottom One past the last row drawn.
            /// @return The texture.
            const Texture& textureForRows(int top, int bottom) const {
                if (m_partsOwner != nullptr) {
                    needRows(top, bottom);
                } else {
                    load();
                }
                return m_texture;
            }

            /// @brief An opaque picture whose pixels are decoded some rows at a time (a large map background).
            /// @param texture The texture, made with Texture::createOpaque.
            /// @param owner The library that decodes it (PictureLibrary::needRows).
            /// @param id Its id in that library.
            static Picture inParts(Texture texture, const PictureLibrary* owner, int id);

            /// @brief Tells whether some rows are still to be decoded.
            bool isInParts() const { return m_partsOwner != nullptr; }

            /// @brief The texture as it is now, without loading or decoding anything (the library fills it through this).
            const Texture& textureAsIs() const { return m_texture; }

            /// @brief Called by the library when every row is decoded.
            void partsDone() { m_partsOwner = nullptr; }

            /// @brief Called by the library when the first rows are decoded.
            void setTopLeft(core::Color color) { m_topLeft = color; }

            /// @brief A picture whose size is known and whose pixels are loaded the first time they are needed.
            /// @param width The width in logical pixels, read from the file's header.
            /// @param height The height in logical pixels.
            /// @param owner The library that loads it (PictureLibrary::loadDeferred).
            /// @param id Its id in that library.
            static Picture deferred(int width, int height, const PictureLibrary* owner, int id);

            /// @brief Tells whether the pixels are still to be loaded.
            bool isDeferred() const { return m_owner != nullptr; }

            /// @brief Tells whether the texture is there, without loading it.
            bool hasTexture() const { return m_texture.isValid(); }

            /// @brief The width in logical pixels.
            /// @return The width.
            int width() const { return m_owner != nullptr ? m_width : m_texture.width(); }

            /// @brief The height in logical pixels.
            /// @return The height.
            int height() const { return m_owner != nullptr ? m_height : m_texture.height(); }

            /// @brief Tells whether a pixel is opaque.
            /// @param x Pixel column.
            /// @param y Pixel row.
            /// @return True when the pixel is opaque.
            bool isOpaque(int x, int y) const;

            /// @brief The top-left pixel (some ground objects mark themselves with it).
            /// @return The colour of that pixel.
            core::Color topLeft() const {
                if (m_partsOwner != nullptr) {
                    needRows(0, 1);
                } else {
                    load();
                }
                return m_topLeft;
            }

        private:
            /// @brief Loads a deferred picture's pixels into this slot.
            void load() const;

            /// @brief Has the library decode some rows of a picture in parts.
            void needRows(int top, int bottom) const;

            const PictureLibrary* m_partsOwner = nullptr; // set while some rows are still to be decoded
            const PictureLibrary* m_owner = nullptr; // set while the pixels are still to be loaded
            int m_id = -1;
            int m_width = 0; // of a deferred picture, from the file's header
            int m_height = 0;
            Texture m_texture;
            std::vector<std::uint64_t> m_opaque; // one bit per pixel, row after row, 64 pixels per word
            core::Color m_topLeft{0, 0, 0, 0};
    };

}

#endif
