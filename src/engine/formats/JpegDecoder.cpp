#include "formats/JpegDecoder.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb_image.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <thread>
#include <vector>

namespace formats {

    namespace {

        static_assert(sizeof(core::Color) == 4, "core::Color must be 4 bytes, RGBA");

        /// @brief Decodes a JPEG whole on this thread.
        /// @param data The JPEG file bytes.
        /// @param bytes How many of them to read.
        /// @param width Gets the width.
        /// @param height Gets the height.
        /// @return The RGBA pixels (free them with stbi_image_free), or null.
        stbi_uc* decodeWhole(const std::uint8_t* data, std::size_t bytes, int& width, int& height) {
            int channels = 0;
            // request 4 channels: stb fills alpha with 255 (opaque) for a JPEG, which is core::Color's RGBA layout
            return stbi_load_from_memory(data, static_cast<int>(bytes), &width, &height, &channels, 4);
        }

        /// @brief Decodes a baseline JPEG in horizontal bands on several threads, when its restart markers allow it.
        /// @details Works when every restart interval is a whole number of block rows and all channels share one
        /// sampling (the large map pictures: 4:4:4 with a restart marker after each block row). Each band is a JPEG of
        /// its own: the original headers with the band height, then that band of entropy data. The pixels match a
        /// whole-picture decode.
        /// @param data The JPEG file bytes.
        /// @return The picture, or nothing when the file cannot be split (the caller then decodes it whole).
        std::optional<Image> decodeInBands(const core::Bytes& data) {
            std::size_t at = 2; // after the start-of-image marker
            std::size_t frame = 0; // where the frame header marker is
            std::size_t scan = 0; // where the entropy data starts
            int width = 0;
            int height = 0;
            int restartInterval = 0; // blocks per restart interval
            int maxH = 1; // largest horizontal and vertical sampling factors
            int maxV = 1;
            bool sameSampling = true;
            while (at + 4 <= data.size() && data[at] == 0xFF) {
                const int marker = data[at + 1];
                const std::size_t length = static_cast<std::size_t>((data[at + 2] << 8) | data[at + 3]);
                if (marker == 0xC0 || marker == 0xC1) { // baseline or extended sequential, Huffman coded
                    if (at + 10 > data.size()) {
                        return std::nullopt;
                    }
                    frame = at;
                    height = (data[at + 5] << 8) | data[at + 6];
                    width = (data[at + 7] << 8) | data[at + 8];
                    const std::size_t channels = data[at + 9];
                    if (at + 11 + 3 * channels > data.size()) {
                        return std::nullopt;
                    }
                    for (std::size_t channel = 0; channel < channels; ++channel) {
                        const int sampling = data[at + 11 + 3 * channel];
                        sameSampling = sameSampling && sampling == data[at + 11];
                        maxH = std::max(maxH, sampling >> 4);
                        maxV = std::max(maxV, sampling & 15);
                    }
                } else if (marker >= 0xC2 && marker <= 0xCF && marker != 0xC4 && marker != 0xC8 && marker != 0xCC) {
                    return std::nullopt; // progressive, lossless or arithmetic coded
                } else if (marker == 0xDD && at + 6 <= data.size()) {
                    restartInterval = (data[at + 4] << 8) | data[at + 5];
                } else if (marker == 0xDA) {
                    scan = at + 2 + length;
                    break;
                }
                at += 2 + length;
            }
            if (frame == 0 || scan == 0 || scan > data.size() || restartInterval == 0 || !sameSampling || width <= 0 ||
                height <= 0) {
                return std::nullopt;
            }
            const int blockWidth = 8 * maxH;
            const int blockHeight = 8 * maxV;
            const int blocksPerRow = (width + blockWidth - 1) / blockWidth;
            const int blockRows = (height + blockHeight - 1) / blockHeight;
            if (restartInterval % blocksPerRow != 0) {
                return std::nullopt;
            }
            const int rowsPerInterval = restartInterval / blocksPerRow;

            // where each restart interval's data starts and ends (at its restart marker, or the marker ending the scan)
            std::vector<std::size_t> starts{scan};
            std::vector<std::size_t> ends;
            for (std::size_t i = scan; i + 1 < data.size(); ++i) {
                if (data[i] != 0xFF) {
                    continue;
                }
                const int marker = data[i + 1];
                if (marker >= 0xD0 && marker <= 0xD7) {
                    ends.push_back(i);
                    starts.push_back(i + 2);
                    ++i;
                } else if (marker != 0x00 && marker != 0xFF) { // 0x00 follows a stuffed 0xFF byte; 0xFF is fill
                    ends.push_back(i);
                    break;
                }
            }
            const int intervals = static_cast<int>(starts.size());
            if (ends.size() != starts.size() || intervals != (blockRows + rowsPerInterval - 1) / rowsPerInterval) {
                return std::nullopt;
            }

            // one band a hardware thread: more bands are no faster (measured on the 6000x6000 maps)
            const int threadCount = static_cast<int>(std::max(1u, std::thread::hardware_concurrency()));
            const int intervalsPerBand = (intervals + std::min(intervals, threadCount) - 1) / std::min(intervals, threadCount);
            const int bands = (intervals + intervalsPerBand - 1) / intervalsPerBand;
            Image image(width, height);
            std::vector<char> decoded(static_cast<std::size_t>(bands), 0);
            const auto decodeBand = [&](int band) {
                const int first = band * intervalsPerBand;
                const int last = std::min(intervals, first + intervalsPerBand); // one past
                const int top = first * rowsPerInterval * blockHeight;
                const int bottom = std::min(height, last * rowsPerInterval * blockHeight);
                core::Bytes part(data.begin(), data.begin() + static_cast<std::ptrdiff_t>(scan));
                part[frame + 5] = static_cast<std::uint8_t>((bottom - top) >> 8);
                part[frame + 6] = static_cast<std::uint8_t>((bottom - top) & 0xFF);
                part.insert(part.end(), data.begin() + static_cast<std::ptrdiff_t>(starts[static_cast<std::size_t>(first)]),
                            data.begin() + static_cast<std::ptrdiff_t>(ends[static_cast<std::size_t>(last - 1)]));
                part.push_back(0xFF); // end of image
                part.push_back(0xD9);
                int partWidth = 0;
                int partHeight = 0;
                stbi_uc* pixels = decodeWhole(part.data(), part.size(), partWidth, partHeight);
                if (pixels != nullptr && partWidth == width && partHeight == bottom - top) {
                    std::memcpy(image.pixels().data() + static_cast<std::size_t>(top) * static_cast<std::size_t>(width), pixels,
                                static_cast<std::size_t>(partWidth) * static_cast<std::size_t>(partHeight) * sizeof(core::Color));
                    decoded[static_cast<std::size_t>(band)] = 1;
                }
                stbi_image_free(pixels);
            };
            std::vector<std::thread> threads;
            for (int band = 1; band < bands; ++band) {
                threads.emplace_back(decodeBand, band);
            }
            decodeBand(0); // this thread takes the first band
            for (std::thread& thread : threads) {
                thread.join();
            }
            if (std::find(decoded.begin(), decoded.end(), 0) != decoded.end()) {
                return std::nullopt;
            }
            return image;
        }

    }

    std::optional<Image> decodeJpeg(const core::Bytes& data) {
        constexpr std::size_t kBandsFromBytes = 1000000; // only large pictures (the maps) are worth the threads
        if (data.size() >= kBandsFromBytes) {
            if (auto image = decodeInBands(data)) {
                return image;
            }
        }
        int width = 0;
        int height = 0;
        stbi_uc* pixels = decodeWhole(data.data(), data.size(), width, height);
        if (pixels == nullptr) {
            return std::nullopt;
        }
        Image image(width, height);
        std::memcpy(image.pixels().data(), pixels, image.pixels().size() * sizeof(core::Color));
        stbi_image_free(pixels);
        return image;
    }

}
