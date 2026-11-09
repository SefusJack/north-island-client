#ifndef NORTH_ISLAND_FORMATS_IMAGE_H
#define NORTH_ISLAND_FORMATS_IMAGE_H

#include "core/Color.h"

#include <cstddef>
#include <vector>

namespace formats {

    /// @brief A decoded picture: RGBA pixels, rows from top to bottom.
    class Image {
        public:
            Image() = default;

            /// @brief Creates a picture filled with one colour.
            /// @param width Width in pixels.
            /// @param height Height in pixels.
            /// @param fill Initial colour of every pixel.
            Image(
                int width,
                int height,
                core::Color fill = {0, 0, 0, 0}
            )
                : m_width(width),
                  m_height(height),
                  m_pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), fill) {}

            /// @brief Width in pixels.
            /// @return The width.
            int width() const { return m_width; }

            /// @brief Height in pixels.
            /// @return The height.
            int height() const { return m_height; }

            /// @brief Whether the picture has no pixels.
            /// @return True when empty.
            bool isEmpty() const { return m_pixels.empty(); }

            /// @brief Mutable pixel access.
            /// @param x Column.
            /// @param y Row (0 is the top).
            /// @return Reference to the pixel.
            core::Color& at(int x, int y) { return m_pixels[index(x, y)]; }

            /// @brief Read-only pixel access.
            /// @param x Column.
            /// @param y Row (0 is the top).
            /// @return Const reference to the pixel.
            const core::Color& at(int x, int y) const { return m_pixels[index(x, y)]; }

            /// @brief All pixels, row by row from the top.
            /// @return Const reference to the pixel vector.
            const std::vector<core::Color>& pixels() const { return m_pixels; }

            /// @brief All pixels, row by row from the top (mutable).
            /// @return Reference to the pixel vector.
            std::vector<core::Color>& pixels() { return m_pixels; }

        private:
            /// @brief Linear index of a pixel.
            /// @param x Column.
            /// @param y Row.
            /// @return Index into the pixel vector.
            std::size_t index(int x, int y) const { return static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) + static_cast<std::size_t>(x); }

            int m_width = 0;
            int m_height = 0;
            std::vector<core::Color> m_pixels;
    };

}

#endif
