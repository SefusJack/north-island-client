#ifndef NORTH_ISLAND_FORMATS_BMP_DECODER_H
#define NORTH_ISLAND_FORMATS_BMP_DECODER_H

#include "core/Bytes.h"
#include "formats/Image.h"

#include <cstddef>
#include <optional>

namespace formats {

    /// @brief Decodes an uncompressed Windows bitmap (1, 4, 8, 24 or 32 bits per pixel, BI_RGB) starting at the BITMAPFILEHEADER; all pixels come out opaque.
    /// @param data File contents.
    /// @return The image, nothing when malformed or unsupported.
    std::optional<Image> decodeBmp(const core::Bytes& data);

    /// @brief Decodes a DIB without file header (BITMAPINFOHEADER first, as inside icons and cursors).
    /// @param data Buffer holding the DIB.
    /// @param start Offset of the BITMAPINFOHEADER.
    /// @param heightOverride Replaces biHeight when non-zero (icons store twice the height).
    /// @param pixelsOffset Where the pixel rows start (0: right after the palette).
    /// @return The image, nothing when malformed or unsupported.
    std::optional<Image> decodeDib(const core::Bytes& data, std::size_t start, int heightOverride = 0, std::size_t pixelsOffset = 0);

}

#endif
