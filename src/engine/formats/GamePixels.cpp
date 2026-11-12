#include "formats/GamePixels.h"

#include "core/Bands.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace formats {

    namespace {

        /// @brief Adds two channel values, clamped to 255.
        /// @param value Channel value.
        /// @param add Amount to add.
        /// @return The saturated sum.
        std::uint8_t addSaturated(std::uint8_t value, std::uint8_t add) {
            return static_cast<std::uint8_t>(std::min(255, value + add)); // channel maximum
        }

        /// @brief A channel lookup table: the output value for each 0..255 input.
        using ChannelTable = std::array<std::uint8_t, 256>;

        /// @brief Builds the table that adds an amount to a channel (clamped to 255), then reduces it to 5 or 6 bits by bit replication.
        /// @details Matches addSaturated followed by the RGB565 round trip (Color::fromRgb565(toRgb565())) for one channel.
        /// @param add Amount added before the reduction (0 for none).
        /// @param bits Kept bits: 5 for red and blue, 6 for green.
        /// @return The table.
        ChannelTable reducedTable(std::uint8_t add, int bits) {
            ChannelTable table{};
            for (int value = 0; value < 256; ++value) {
                const int sum = std::min(255, value + add); // channel maximum
                const int kept = sum >> (8 - bits);
                table[static_cast<std::size_t>(value)] = static_cast<std::uint8_t>((kept << (8 - bits)) | (kept >> (2 * bits - 8)));
            }
            return table;
        }

        /// @brief Computes one RGB565 field of the alpha fill.
        /// @details Original: one field of rodraw2's ro_Clipper_Alpha_Fill, (fill + 64 - dest) * alpha >> 8, plus dest, less alpha / 4 (16-bit lanes).
        /// @param fill Fill field value.
        /// @param dest Destination field value.
        /// @param alpha Fill strength (0..255).
        /// @return The new field value.
        int fillField(int fill, int dest, int alpha) {
            // 0x40: bias of the original's arithmetic; 0xFFFF: 16-bit lane mask
            const std::uint16_t scaled = static_cast<std::uint16_t>(((fill + 0x40 - dest) * alpha) & 0xFFFF);
            // 8: lane shift of the multiply; 2: alpha / 4 correction
            return static_cast<std::uint16_t>((scaled >> 8) + dest - (alpha >> 2));
        }

        /// @brief Shifts right by 8 the way the generated imul/shr code does on a negative value (rounds the magnitude up).
        /// @details Original: blue lane of DXR_BLEND_SRCALPHA1_ADD_INVSRCALPHA2, (fill - lane) * alpha >> 8.
        /// @param value Product to shift.
        /// @return The shifted value.
        int floorShift8(int value) { return value >= 0 ? value >> 8 : -((-value + 255) >> 8); } // 8: shift of the 8-bit alpha scale; 255: rounding bias

        constexpr std::size_t kBandsFromPixels = 1000000; // pictures this large (the maps) are worked on in bands on several threads

    }

    void addColor(Image& image, core::Color colorAdd) {
        if (colorAdd.r == 0 && colorAdd.g == 0 && colorAdd.b == 0) {
            return; // nothing to add
        }
        core::Color* const pixels = image.pixels().data();
        core::inBands(image.pixels().size(), kBandsFromPixels, [&](std::size_t begin, std::size_t end) {
            for (std::size_t i = begin; i < end; ++i) {
                core::Color& pixel = pixels[i];
                if ((pixel.r | pixel.g | pixel.b) < 4) { // near-black threshold left untouched
                    continue;
                }
                pixel.r = addSaturated(pixel.r, colorAdd.r);
                pixel.g = addSaturated(pixel.g, colorAdd.g);
                pixel.b = addSaturated(pixel.b, colorAdd.b);
            }
        });
    }

    void applyPictureRules(Image& image, core::Color colorAdd) {
        // per channel: the tint added, then the 16-bit reduction, looked up instead of computed per pixel
        const ChannelTable red = reducedTable(colorAdd.r, 5);
        const ChannelTable green = reducedTable(colorAdd.g, 6);
        const ChannelTable blue = reducedTable(colorAdd.b, 5);
        const std::uint8_t markerBlue = reducedTable(0, 5)[8]; // the near-black marker (0, 0, 8) reduced without the tint
        core::Color* const pixels = image.pixels().data();
        core::inBands(image.pixels().size(), kBandsFromPixels, [&](std::size_t begin, std::size_t end) {
            for (std::size_t i = begin; i < end; ++i) {
                core::Color& pixel = pixels[i];
                if (pixel.r == kTransparentKey.r && pixel.g == kTransparentKey.g && pixel.b == kTransparentKey.b) {
                    pixel = {0, 0, 0, 0};
                    continue;
                }
                // 8: darkness threshold per channel; dark pixels get blue 8 so they stay visible
                if (pixel.r < 8 && pixel.g < 8 && pixel.b < 8) {
                    pixel.b = 8;
                }
                if (pixel.r == 0 && pixel.g == 0 && pixel.b == 8) { // the near-black marker colour (0, 0, 8): not tinted
                    pixel = {0, 0, markerBlue, 255}; // 255: opaque
                    continue;
                }
                pixel = {red[pixel.r], green[pixel.g], blue[pixel.b], 255}; // 255: opaque
            }
        });
    }

    void quantizeTo16Bit(Image& image) {
        const ChannelTable five = reducedTable(0, 5);
        const ChannelTable six = reducedTable(0, 6);
        core::Color* const pixels = image.pixels().data();
        core::inBands(image.pixels().size(), kBandsFromPixels, [&](std::size_t begin, std::size_t end) {
            for (std::size_t i = begin; i < end; ++i) {
                core::Color& pixel = pixels[i];
                pixel = {five[pixel.r], six[pixel.g], five[pixel.b], pixel.a};
            }
        });
    }

    std::vector<std::uint64_t> opaqueBits(const Image& image) {
        const std::vector<core::Color>& pixels = image.pixels();
        std::vector<std::uint64_t> bits((pixels.size() + 63) / 64, 0);
        core::inBands(bits.size(), kBandsFromPixels / 64, [&](std::size_t begin, std::size_t end) {
            for (std::size_t word = begin; word < end; ++word) {
                const std::size_t first = word * 64;
                const std::size_t count = std::min<std::size_t>(64, pixels.size() - first);
                std::uint64_t value = 0;
                for (std::size_t bit = 0; bit < count; ++bit) {
                    value |= static_cast<std::uint64_t>(pixels[first + bit].a != 0) << bit;
                }
                bits[word] = value;
            }
        });
        return bits;
    }

    void alphaFill16(Image& image, core::Color fill) {
        const std::uint16_t color = fill.toRgb565();
        for (core::Color& pixel : image.pixels()) {
            const std::uint16_t dest = pixel.toRgb565();
            // RGB565 layout: red bits 15..11 (mask 0x1F), green bits 10..5 (mask 0x3F), blue bits 4..0 (mask 0x1F)
            const int r = fillField(color >> 11, dest >> 11, fill.a);
            const int g = fillField((color >> 5) & 0x3F, (dest >> 5) & 0x3F, fill.a);
            const int b = fillField(color & 0x1F, dest & 0x1F, fill.a);
            const std::uint8_t alpha = pixel.a;
            // RGB565 layout: red bits 15..11 (mask 0x1F), green bits 10..5 (mask 0x3F), blue bits 4..0 (mask 0x1F)
            pixel = core::Color::fromRgb565(static_cast<std::uint16_t>(((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F)));
            pixel.a = alpha;
        }
    }

    void fillAlpha16(Image& image, core::Color fill) {
        const int alpha = fill.a;
        for (core::Color& pixel : image.pixels()) {
            // lanes: the 565 fields with the low bits of an 8-bit lane zeroed (red/blue drop 3 bits, green 2); 255 is full alpha; >> 8 scales back
            const int redLane = (pixel.r >> 3) << 3;
            const int greenLane = (pixel.g >> 2) << 2;
            const int blueLane = (pixel.b >> 3) << 3;
            const int red = (fill.r * alpha + redLane * (255 - alpha)) >> 8;
            const int green = (fill.g * alpha + greenLane * (255 - alpha)) >> 8;
            const int blue = floorShift8((fill.b - blueLane) * alpha) + blueLane;
            const std::uint8_t opacity = pixel.a;
            // lanes back to RGB565 fields: drop 3/2/3 low bits, place at bit 11/5/0
            pixel = core::Color::fromRgb565(static_cast<std::uint16_t>((((red & 0xFF) >> 3) << 11) | (((green & 0xFF) >> 2) << 5) | ((blue & 0xFF) >> 3)));
            pixel.a = opacity;
        }
    }

}
