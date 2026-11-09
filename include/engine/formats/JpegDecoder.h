#ifndef NORTH_ISLAND_FORMATS_JPEG_DECODER_H
#define NORTH_ISLAND_FORMATS_JPEG_DECODER_H

#include "core/Bytes.h"
#include "formats/Image.h"

#include <optional>

namespace formats {

    /// @brief Decodes a baseline or progressive JPEG into opaque pixels.
    /// @param data Raw JPEG file bytes.
    /// @return The image, or nothing when decoding fails.
    std::optional<Image> decodeJpeg(const core::Bytes& data);

}

#endif
