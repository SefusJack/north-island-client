#ifndef NORTH_ISLAND_FORMATS_PNG_CODEC_H
#define NORTH_ISLAND_FORMATS_PNG_CODEC_H

#include "core/Bytes.h"
#include "formats/Image.h"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace formats {

    /// @brief Decodes a PNG (RGBA).
    /// @param data Raw PNG bytes.
    /// @return The image, or nothing when decoding fails.
    std::optional<Image> decodePng(const core::Bytes& data);

    /// @brief Encodes an image as a PNG with alpha.
    /// @param image Source image.
    /// @return The PNG bytes.
    core::Bytes encodePng(const Image& image);

    /// @brief An 8-bit paletted picture: palette indices, rows top-down, and the palette.
    /// @details Used by the sprite export tool's frames and the mod packs' HD sprite frames, whose indices take the game's colours as the original frames' do.
    struct IndexedImage {
        int width = 0;
        int height = 0;
        std::vector<std::uint8_t> pixels;
        std::array<core::Color, 256> palette{}; // 256 = entries of an 8-bit palette
    };

    /// @brief Decodes an 8-bit paletted PNG (colour type 3, not interlaced) keeping its indices.
    /// @param data Raw PNG bytes.
    /// @return The indexed image, or nothing for any other kind of PNG.
    std::optional<IndexedImage> decodeIndexedPng(const core::Bytes& data);

    /// @brief Encodes an indexed image as an 8-bit paletted PNG, index 0 transparent.
    /// @param image Source image.
    /// @return The PNG bytes.
    core::Bytes encodeIndexedPng(const IndexedImage& image);

}

#endif
