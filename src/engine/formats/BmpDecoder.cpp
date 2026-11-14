#include "formats/BmpDecoder.h"

#include "core/ByteReader.h"

#include <vector>

namespace formats {

    namespace {

        constexpr std::uint32_t kBiRgb = 0; // BMP compression 0: BI_RGB, uncompressed
        constexpr std::uint32_t kBiBitfields = 3; // BMP compression 3: BI_BITFIELDS, uncompressed with channel masks

    }

    std::optional<Image> decodeDib(const core::Bytes& data, std::size_t start, int heightOverride, std::size_t pixelsOffset) {
        core::ByteReader reader(data, start);
        const std::uint32_t headerSize = reader.u32();
        const int width = reader.i32();
        const int storedHeight = reader.i32();
        reader.u16();
        const int bitCount = reader.u16();
        const std::uint32_t compression = reader.u32();
        reader.u32();
        reader.u32();
        reader.u32();
        std::uint32_t colorsUsed = reader.u32();
        if (reader.failed() || headerSize < 40 || width <= 0 || storedHeight == 0) { // BITMAPINFOHEADER is 40 bytes
            return std::nullopt;
        }
        if (compression != kBiRgb &&
            !(compression == kBiBitfields && (bitCount == 16 || bitCount == 32))) { // bit-field masks only for 16 and 32 bits per pixel
            return std::nullopt;
        }

        const bool bottomUp = storedHeight > 0;
        const int height = heightOverride != 0 ? heightOverride : (bottomUp ? storedHeight : -storedHeight);

        // default channel masks: RGB555 for 16 bits, 0x00RRGGBB for 32 bits
        std::uint32_t masks[3] = {0x7C00, 0x03E0, 0x001F}; // RGB555 red bits 10..14, green bits 5..9, blue bits 0..4
        if (bitCount == 32) {
            masks[0] = 0x00FF0000; // red: bits 16..23
            masks[1] = 0x0000FF00; // green: bits 8..15
            masks[2] = 0x000000FF; // blue: bits 0..7
        }
        reader.seek(start + headerSize);
        if (compression == kBiBitfields) {
            for (std::uint32_t& mask : masks) {
                mask = reader.u32();
            }
        }

        std::vector<core::Color> palette;
        if (bitCount <= 8) {
            if (colorsUsed == 0) {
                colorsUsed = 1u << bitCount;
            }
            for (std::uint32_t i = 0; i < colorsUsed; ++i) {
                const std::uint8_t b = reader.u8();
                const std::uint8_t g = reader.u8();
                const std::uint8_t r = reader.u8();
                reader.u8();
                palette.push_back({r, g, b, 255}); // opaque
            }
        }
        if (reader.failed()) {
            return std::nullopt;
        }

        const std::size_t rowBytes =
            ((static_cast<std::size_t>(width) * static_cast<std::size_t>(bitCount) + 31) / 32) * 4; // DIB rows are padded to 32-bit (4-byte) boundaries
        const std::size_t pixelStart = pixelsOffset != 0 ? pixelsOffset : reader.position();
        if (pixelStart + rowBytes * static_cast<std::size_t>(height) > data.size()) {
            return std::nullopt;
        }

        const auto channel = [](std::uint32_t value, std::uint32_t mask) -> std::uint8_t {
            if (mask == 0) {
                return 0;
            }
            int shift = 0;
            while (((mask >> shift) & 1u) == 0) {
                ++shift;
            }
            const std::uint32_t bits = mask >> shift;
            const std::uint32_t v = (value & mask) >> shift;
            return static_cast<std::uint8_t>(bits == 0 ? 0 : v * 255 / bits); // scale the channel to 0..255
        };

        Image image(width, height);
        // The two kinds the game's pictures come in (24 bits, and 8 bits with a palette) are read a row at a time into whole
        // 32-bit pixels (bytes r, g, b, a: red is the low byte);
        static_assert(sizeof(core::Color) == 4, "core::Color must be four bytes: r, g, b, a");
        if (bitCount == 24 || bitCount == 8) {
            std::uint32_t entries[256];
            for (std::size_t i = 0; i < 256; ++i) {
                const core::Color color = i < palette.size() ? palette[i] : core::Color{0, 0, 0, 255}; // opaque black for a missing entry
                entries[i] = static_cast<std::uint32_t>(color.r) | (static_cast<std::uint32_t>(color.g) << 8) |
                             (static_cast<std::uint32_t>(color.b) << 16) | 0xFF000000u;
            }
            auto* out = reinterpret_cast<std::uint32_t*>(image.pixels().data());
            for (int y = 0; y < height; ++y) {
                const int sourceRow = bottomUp ? height - 1 - y : y;
                const std::uint8_t* from = data.data() + pixelStart + static_cast<std::size_t>(sourceRow) * rowBytes;
                if (bitCount == 24) {
                    for (int x = 0; x < width; ++x, from += 3) { // bytes are stored B, G, R
                        *out++ = static_cast<std::uint32_t>(from[2]) | (static_cast<std::uint32_t>(from[1]) << 8) |
                                 (static_cast<std::uint32_t>(from[0]) << 16) | 0xFF000000u;
                    }
                } else {
                    for (int x = 0; x < width; ++x) {
                        *out++ = entries[from[x]];
                    }
                }
            }
            return image;
        }
        for (int y = 0; y < height; ++y) {
            const int sourceRow = bottomUp ? height - 1 - y : y;
            const std::size_t row = pixelStart + static_cast<std::size_t>(sourceRow) * rowBytes;
            for (int x = 0; x < width; ++x) {
                core::Color color;
                switch (bitCount) {
                    case 1: // 1 bit per pixel, palette
                    case 4: // 4 bits per pixel, palette
                    case 8: { // 8 bits per pixel, palette
                        const std::size_t bitOffset = static_cast<std::size_t>(x) * static_cast<std::size_t>(bitCount);
                        const std::uint8_t byte = data[row + bitOffset / 8]; // 8 bits per byte
                        const int shift = 8 - bitCount - static_cast<int>(bitOffset % 8); // leftmost pixel in the high bits
                        const std::size_t entry = (byte >> shift) & ((1u << bitCount) - 1);
                        color = entry < palette.size() ? palette[entry] : core::Color{0, 0, 0, 255}; // opaque black for a missing entry
                        break;
                    }
                    case 16: { // 16 bits per pixel, bit-field masks
                        const std::uint32_t value = data[row + x * 2u] | (data[row + x * 2u + 1] << 8); // 2 bytes per pixel, little-endian
                        color = {channel(value, masks[0]), channel(value, masks[1]), channel(value, masks[2]), 255}; // opaque
                        break;
                    }
                    case 24: { // 24 bits per pixel, B G R bytes
                        const std::size_t p = row + static_cast<std::size_t>(x) * 3; // 3 bytes per pixel
                        color = {data[p + 2], data[p + 1], data[p], 255}; // bytes are stored B, G, R; opaque
                        break;
                    }
                    case 32: { // 32 bits per pixel, bit-field masks
                        const std::size_t p = row + static_cast<std::size_t>(x) * 4; // 4 bytes per pixel
                        // little-endian 32-bit value from 4 bytes
                        const std::uint32_t value = data[p] | (data[p + 1] << 8) | (data[p + 2] << 16) | (static_cast<std::uint32_t>(data[p + 3]) << 24);
                        color = {channel(value, masks[0]), channel(value, masks[1]), channel(value, masks[2]), 255}; // opaque
                        break;
                    }
                    default: // unsupported bit depth
                        return std::nullopt;
                }
                image.at(x, y) = color;
            }
        }
        return image;
    }

    std::optional<Image> decodeBmp(const core::Bytes& data) {
        core::ByteReader reader(data);
        if (reader.u8() != 'B' || reader.u8() != 'M') {
            return std::nullopt;
        }
        reader.u32();
        reader.u32();
        const std::uint32_t pixelsOffset = reader.u32();
        if (reader.failed()) {
            return std::nullopt;
        }
        return decodeDib(data, 14, 0, pixelsOffset); // BITMAPFILEHEADER is 14 bytes
    }

}
