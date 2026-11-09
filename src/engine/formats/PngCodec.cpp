#include "formats/PngCodec.h"

#define STBI_NO_STDIO
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "stb_image_write.h"

#include <algorithm>
#include <cstdlib>
#include <string>

namespace formats {

    namespace {

        // the 8-byte PNG file signature (0x89 'P' 'N' 'G' CR LF SUB LF)
        constexpr std::uint8_t kSignature[8] = {137, 80, 78, 71, 13, 10, 26, 10};

        /// @brief CRC-32 of a chunk (type and body) as PNG stores it.
        /// @param data Buffer holding the chunk.
        /// @param from Offset where the checksummed bytes start; runs to the end of the buffer.
        /// @return The CRC.
        std::uint32_t crc32(const core::Bytes& data, std::size_t from) {
            static const auto table = [] {
                std::array<std::uint32_t, 256> entries{}; // one entry per byte value
                for (std::uint32_t n = 0; n < 256; ++n) { // 256 byte values
                    std::uint32_t c = n;
                    for (int k = 0; k < 8; ++k) { // 8 bits per byte
                        c = (c & 1) != 0 ? 0xEDB88320u ^ (c >> 1) : c >> 1; // 0xEDB88320: reflected CRC-32 polynomial
                    }
                    entries[n] = c;
                }
                return entries;
            }();
            std::uint32_t crc = 0xFFFFFFFFu; // CRC-32 initial value
            for (std::size_t i = from; i < data.size(); ++i) {
                crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8); // 0xFF: low byte index; 8: byte shift
            }
            return ~crc;
        }

        /// @brief Reads a big-endian u32.
        /// @param data Buffer.
        /// @param at Offset of the value.
        /// @return The value.
        std::uint32_t readU32(const core::Bytes& data, std::size_t at) {
            // big-endian: byte shifts 24, 16, 8; bytes at +1, +2, +3
            return (static_cast<std::uint32_t>(data[at]) << 24) | (static_cast<std::uint32_t>(data[at + 1]) << 16) |
                   (static_cast<std::uint32_t>(data[at + 2]) << 8) | data[at + 3];
        }

        /// @brief Appends a big-endian u32.
        /// @param out Buffer to append to.
        /// @param value Value to write.
        void putU32(core::Bytes& out, std::uint32_t value) {
            // big-endian: byte shifts 24, 16, 8
            out.insert(
                out.end(),
                {static_cast<std::uint8_t>(value >> 24),
                 static_cast<std::uint8_t>(value >> 16),
                 static_cast<std::uint8_t>(value >> 8),
                 static_cast<std::uint8_t>(value)}
            );
        }

        /// @brief Appends one PNG chunk (length, type, body, CRC).
        /// @param out Buffer to append to.
        /// @param type Four-letter chunk type.
        /// @param body Chunk data.
        void putChunk(core::Bytes& out, const char* type, const core::Bytes& body) {
            putU32(out, static_cast<std::uint32_t>(body.size()));
            const std::size_t start = out.size();
            out.insert(out.end(), type, type + 4); // chunk type is 4 letters
            out.insert(out.end(), body.begin(), body.end());
            putU32(out, crc32(out, start));
        }

    }

    std::optional<Image> decodePng(const core::Bytes& data) {
        int width = 0;
        int height = 0;
        int channels = 0;
        stbi_uc* pixels = stbi_load_from_memory(data.data(), static_cast<int>(data.size()), &width, &height, &channels, 4); // request 4 channels (RGBA)
        if (pixels == nullptr) {
            return std::nullopt;
        }
        Image image(width, height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const stbi_uc* p =
                    pixels + (static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)) * 4; // 4 bytes per RGBA pixel
                image.at(x, y) = {p[0], p[1], p[2], p[3]}; // R, G, B, A bytes
            }
        }
        stbi_image_free(pixels);
        return image;
    }

    core::Bytes encodePng(const Image& image) {
        core::Bytes out;
        if (image.isEmpty()) {
            return out;
        }
        std::vector<unsigned char> rgba;
        rgba.reserve(image.pixels().size() * 4); // 4 bytes per RGBA pixel
        for (const core::Color& c : image.pixels()) {
            rgba.insert(rgba.end(), {c.r, c.g, c.b, c.a});
        }
        stbi_write_png_to_func(
            [](void* context, void* data, int size) {
                auto& bytes = *static_cast<core::Bytes*>(context);
                const auto* begin = static_cast<const std::uint8_t*>(data);
                bytes.insert(bytes.end(), begin, begin + size);
            },
            &out,
            image.width(),
            image.height(),
            4,
            rgba.data(),
            image.width() * 4
        ); // 4 components per pixel; row stride = width * 4 bytes
        return out;
    }

    std::optional<IndexedImage> decodeIndexedPng(const core::Bytes& data) {
        if (data.size() < 8 || !std::equal(kSignature, kSignature + 8, data.begin())) { // 8: signature length
            return std::nullopt;
        }
        IndexedImage image;
        bool header = false;
        core::Bytes compressed;
        // chunks start after the 8-byte signature; each has 12 bytes of overhead (length 4, type 4, CRC 4)
        for (std::size_t at = 8; at + 12 <= data.size();) {
            const std::uint32_t length = readU32(data, at);
            if (length > data.size() - at - 12) {
                return std::nullopt;
            }
            // type at +4..+8, body at +8
            const std::string type(data.begin() + static_cast<std::ptrdiff_t>(at + 4), data.begin() + static_cast<std::ptrdiff_t>(at + 8));
            const std::size_t body = at + 8;
            if (type == "IHDR" && length >= 13) { // the IHDR body is 13 bytes
                image.width = static_cast<int>(readU32(data, body));
                image.height = static_cast<int>(readU32(data, body + 4)); // height follows the 4-byte width
                // IHDR offsets: 8 bit depth (8), 9 colour type (3 = paletted), 12 interlace (0 = none)
                if (data[body + 8] != 8 || data[body + 9] != 3 || data[body + 12] != 0) {
                    return std::nullopt;
                }
                header = true;
            } else if (type == "PLTE") {
                // 3 bytes (R, G, B) per palette entry; at most 256 entries; alpha 255 until tRNS says otherwise
                for (std::size_t i = 0; i < std::min<std::size_t>(length / 3, 256); ++i) {
                    image.palette[i] = {data[body + i * 3], data[body + i * 3 + 1], data[body + i * 3 + 2], 255};
                }
            } else if (type == "tRNS") {
                for (std::size_t i = 0; i < std::min<std::size_t>(length, 256); ++i) { // one alpha byte per palette entry, at most 256
                    image.palette[i].a = data[body + i];
                }
            } else if (type == "IDAT") {
                const auto begin = data.begin() + static_cast<std::ptrdiff_t>(body);
                compressed.insert(compressed.end(), begin, begin + static_cast<std::ptrdiff_t>(length));
            } else if (type == "IEND") {
                break;
            }
            at += 12 + length; // 12 bytes of chunk overhead
        }
        if (!header || image.width <= 0 || image.height <= 0 || image.width > 16384 || image.height > 16384) { // largest accepted picture side in pixels
            return std::nullopt;
        }
        int size = 0;
        char* raw = stbi_zlib_decode_malloc(reinterpret_cast<const char*>(compressed.data()), static_cast<int>(compressed.size()), &size);
        const std::size_t stride = static_cast<std::size_t>(image.width);
        const std::size_t height = static_cast<std::size_t>(image.height);
        if (raw == nullptr || static_cast<std::size_t>(size) < (stride + 1) * height) { // each row is a filter byte plus width index bytes
            std::free(raw);
            return std::nullopt;
        }
        image.pixels.resize(stride * height);
        const auto* in = reinterpret_cast<const std::uint8_t*>(raw);
        for (std::size_t y = 0; y < height; ++y) {
            const std::uint8_t filter = in[y * (stride + 1)];
            const std::uint8_t* line = in + y * (stride + 1) + 1;
            const std::size_t row = y * stride;
            const bool up = y > 0;
            for (std::size_t x = 0; x < stride; ++x) {
                const int a = x > 0 ? image.pixels[row + x - 1] : 0;
                const int b = up ? image.pixels[row - stride + x] : 0;
                const int c = x > 0 && up ? image.pixels[row - stride + x - 1] : 0;
                int predicted = 0;
                switch (filter) {
                    case 0: break; // PNG filter 0: None
                    case 1: predicted = a; break; // PNG filter 1: Sub (left neighbour)
                    case 2: predicted = b; break; // PNG filter 2: Up (neighbour above)
                    case 3: predicted = (a + b) / 2; break; // PNG filter 3: Average of left and above
                    case 4: { // PNG filter 4: Paeth predictor
                        const int p = a + b - c;
                        const int pa = std::abs(p - a);
                        const int pb = std::abs(p - b);
                        const int pc = std::abs(p - c);
                        predicted = pa <= pb && pa <= pc ? a : pb <= pc ? b : c;
                        break;
                    }
                    default: // unknown filter type: reject the picture
                        std::free(raw);
                        return std::nullopt;
                }
                image.pixels[row + x] = static_cast<std::uint8_t>(line[x] + predicted);
            }
        }
        std::free(raw);
        return image;
    }

    core::Bytes encodeIndexedPng(const IndexedImage& image) {
        if (image.width <= 0 || image.height <= 0 || image.pixels.size() != static_cast<std::size_t>(image.width) * static_cast<std::size_t>(image.height)) {
            return {};
        }
        core::Bytes out(kSignature, kSignature + 8); // 8: signature length
        core::Bytes header;
        putU32(header, static_cast<std::uint32_t>(image.width));
        putU32(header, static_cast<std::uint32_t>(image.height));
        header.insert(header.end(), {8, 3, 0, 0, 0}); // IHDR: bit depth 8, colour type 3 (paletted), compression 0, filter 0, interlace 0
        putChunk(out, "IHDR", header);
        core::Bytes palette;
        for (const core::Color& c : image.palette) {
            palette.insert(palette.end(), {c.r, c.g, c.b});
        }
        putChunk(out, "PLTE", palette);
        putChunk(out, "tRNS", core::Bytes{0}); // palette index 0 gets alpha 0 (transparent)
        std::vector<unsigned char> rows;
        rows.reserve((static_cast<std::size_t>(image.width) + 1) * static_cast<std::size_t>(image.height));
        for (int y = 0; y < image.height; ++y) {
            rows.push_back(0); // filter type 0 (None) for each row
            const auto start = image.pixels.begin() + static_cast<std::ptrdiff_t>(y) * image.width;
            rows.insert(rows.end(), start, start + image.width);
        }
        int length = 0;
        unsigned char* compressed = stbi_zlib_compress(rows.data(), static_cast<int>(rows.size()), &length, 8); // zlib compression quality
        if (compressed == nullptr) {
            return {};
        }
        putChunk(out, "IDAT", core::Bytes(compressed, compressed + length));
        STBIW_FREE(compressed);
        putChunk(out, "IEND", {});
        return out;
    }

}
