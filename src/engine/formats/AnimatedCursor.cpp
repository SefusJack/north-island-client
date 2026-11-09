#include "formats/AnimatedCursor.h"

#include "core/ByteReader.h"
#include "core/FileSystem.h"
#include "formats/BmpDecoder.h"

#include <string>

namespace formats {

    namespace {

        /// @brief Decodes the image of one icon directory entry: a DIB with twice the height (colour rows, then a 1-bit AND mask), alpha from the mask or from the 32-bit pixels.
        /// @param data File contents.
        /// @param start Offset of the entry's DIB.
        /// @return The image, nothing when malformed.
        std::optional<Image> decodeIconImage(const core::Bytes& data, std::size_t start) {
            core::ByteReader reader(data, start);
            const std::uint32_t headerSize = reader.u32();
            const int width = reader.i32();
            const int doubledHeight = reader.i32();
            reader.u16();
            const int bitCount = reader.u16();
            reader.u32();
            reader.u32();
            reader.u32();
            reader.u32();
            std::uint32_t colorsUsed = reader.u32();
            if (reader.failed() || width <= 0 || doubledHeight <= 0) {
                return std::nullopt;
            }
            const int height = doubledHeight / 2;
            if (bitCount <= 8 && colorsUsed == 0) {
                colorsUsed = 1u << bitCount;
            }
            const std::size_t paletteBytes = bitCount <= 8 ? colorsUsed * 4 : 0; // palette only up to 8 bits per pixel; 4 bytes per palette entry
            const std::size_t colorStart = start + headerSize + paletteBytes;
            const std::size_t colorRowBytes =
                ((static_cast<std::size_t>(width) * static_cast<std::size_t>(bitCount) + 31) / 32) * 4; // DIB rows are padded to 32-bit (4-byte) boundaries
            const std::size_t maskStart = colorStart + colorRowBytes * static_cast<std::size_t>(height);
            const std::size_t maskRowBytes = ((static_cast<std::size_t>(width) + 31) / 32) * 4; // 1-bit mask rows padded to 32-bit (4-byte) boundaries

            auto image = decodeDib(data, start, height, colorStart);
            if (!image) {
                return std::nullopt;
            }

            bool hasAlpha = false;
            if (bitCount == 32) { // 32-bit pixels carry their own alpha
                for (int y = 0; y < height && !hasAlpha; ++y) {
                    const std::size_t row = colorStart + static_cast<std::size_t>(height - 1 - y) * colorRowBytes;
                    for (int x = 0; x < width; ++x) {
                        if (data[row + static_cast<std::size_t>(x) * 4 + 3] != 0) { // 4 bytes per pixel; byte 3 is alpha
                            hasAlpha = true;
                            break;
                        }
                    }
                }
            }
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    std::uint8_t alpha = 255; // opaque
                    if (hasAlpha) {
                        const std::size_t row = colorStart + static_cast<std::size_t>(height - 1 - y) * colorRowBytes;
                        alpha = data[row + static_cast<std::size_t>(x) * 4 + 3]; // 4 bytes per pixel; byte 3 is alpha
                    } else {
                        const std::size_t row = maskStart + static_cast<std::size_t>(height - 1 - y) * maskRowBytes;
                        // mask bits: 8 pixels per byte, leftmost pixel in the top bit (0x80)
                        if (row + static_cast<std::size_t>(x) / 8 < data.size()) {
                            const bool transparent = (data[row + static_cast<std::size_t>(x) / 8] & (0x80 >> (x % 8))) != 0;
                            alpha = transparent ? 0 : 255; // opaque
                        }
                    }
                    image->at(x, y).a = alpha;
                }
            }
            return image;
        }

        /// @brief Reads a four-character RIFF chunk id.
        /// @param reader Reader positioned at the id.
        /// @return The id text.
        std::string fourCc(core::ByteReader& reader) { return reader.text(4); } // a four-character code is 4 bytes

    }

    std::optional<CursorFrame> AnimatedCursor::parseIcon(const core::Bytes& data) {
        core::ByteReader reader(data);
        reader.u16();
        const int type = reader.u16();
        const int count = reader.u16();
        if (reader.failed() || (type != 1 && type != 2) || count < 1) { // resource type 1 = icon, 2 = cursor
            return std::nullopt;
        }
        reader.u8();
        reader.u8();
        reader.u8();
        reader.u8();
        const int hotspotX = reader.u16();
        const int hotspotY = reader.u16();
        reader.u32();
        const std::uint32_t offset = reader.u32();
        if (reader.failed()) {
            return std::nullopt;
        }
        auto image = decodeIconImage(data, offset);
        if (!image) {
            return std::nullopt;
        }
        CursorFrame frame;
        frame.image = std::move(*image);
        if (type == 2) { // resource type 2 = cursor (has a hotspot)
            frame.hotspot = {hotspotX, hotspotY};
        }
        return frame;
    }

    std::optional<AnimatedCursor> AnimatedCursor::parse(const core::Bytes& data) {
        core::ByteReader reader(data);
        if (fourCc(reader) != "RIFF") {
            auto single = parseIcon(data);
            if (!single) {
                return std::nullopt;
            }
            AnimatedCursor cursor;
            cursor.m_frames.push_back(std::move(*single));
            cursor.m_steps.push_back({0, 0});
            return cursor;
        }
        reader.u32();
        if (fourCc(reader) != "ACON") {
            return std::nullopt;
        }

        AnimatedCursor cursor;
        int defaultJiffies = 10; // default display time in jiffies (1/60 s) when the file gives none
        std::uint32_t stepCount = 0;
        std::vector<std::uint32_t> rates;
        std::vector<std::uint32_t> sequence;
        while (reader.remaining() >= 8) { // a chunk header is 8 bytes (id + size)
            const std::string id = fourCc(reader);
            const std::uint32_t size = reader.u32();
            const std::size_t chunkStart = reader.position();
            if (id == "anih") {
                reader.u32();
                reader.u32();
                stepCount = reader.u32();
                // 16 bytes = four u32 fields
                reader.skip(16);
                defaultJiffies = static_cast<int>(reader.u32());
            } else if (id == "rate") {
                for (std::uint32_t i = 0; i < size / 4; ++i) { // one u32 (4 bytes) per step
                    rates.push_back(reader.u32());
                }
            } else if (id == "seq ") {
                for (std::uint32_t i = 0; i < size / 4; ++i) { // one u32 (4 bytes) per step
                    sequence.push_back(reader.u32());
                }
            } else if (id == "LIST") {
                if (fourCc(reader) == "fram") {
                    while (reader.position() + 8 <= chunkStart + size) { // a sub-chunk header is 8 bytes
                        const std::string sub = fourCc(reader);
                        const std::uint32_t subSize = reader.u32();
                        const std::size_t subStart = reader.position();
                        if (sub == "icon") {
                            core::Bytes icon = reader.bytes(subSize);
                            if (auto frame = parseIcon(icon)) {
                                cursor.m_frames.push_back(std::move(*frame));
                            }
                        }
                        reader.seek(subStart + subSize + (subSize & 1)); // RIFF chunks are padded to even sizes
                    }
                }
            }
            reader.seek(chunkStart + size + (size & 1)); // RIFF chunks are padded to even sizes
            if (reader.failed()) {
                break;
            }
        }
        if (cursor.m_frames.empty()) {
            return std::nullopt;
        }
        if (stepCount == 0) {
            stepCount = static_cast<std::uint32_t>(cursor.m_frames.size());
        }
        for (std::uint32_t i = 0; i < stepCount; ++i) {
            Step step;
            step.frame = i < sequence.size() ? sequence[i] : i;
            if (step.frame >= cursor.m_frames.size()) {
                step.frame = 0;
            }
            const int jiffies = i < rates.size() ? static_cast<int>(rates[i]) : defaultJiffies;
            // 1000 ms per second, 60 jiffies per second
            step.durationMs = jiffies * 1000 / 60;
            cursor.m_steps.push_back(step);
        }
        return cursor;
    }

    std::optional<AnimatedCursor> AnimatedCursor::load(const std::filesystem::path& path) {
        const auto data = core::readFile(path);
        if (!data) {
            return std::nullopt;
        }
        return parse(*data);
    }

}
