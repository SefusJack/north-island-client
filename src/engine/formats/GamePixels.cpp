#include "formats/GamePixels.h"

#include <algorithm>

namespace formats {

    namespace {

        /// @brief Adds two channel values, clamped to 255.
        /// @param value Channel value.
        /// @param add Amount to add.
        /// @return The saturated sum.
        std::uint8_t addSaturated(std::uint8_t value, std::uint8_t add) {
            return static_cast<std::uint8_t>(std::min(255, value + add)); // channel maximum
        }

        /// @brief Reduces a colour to RGB565 precision, keeping its alpha.
        /// @param color Source colour.
        /// @return The reduced colour.
        core::Color to16Bit(core::Color color) {
            core::Color reduced = core::Color::fromRgb565(color.toRgb565());
            reduced.a = color.a;
            return reduced;
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

    }

    void addColor(Image& image, core::Color colorAdd) {
        for (core::Color& pixel : image.pixels()) {
            if ((pixel.r | pixel.g | pixel.b) < 4) { // near-black threshold left untouched
                continue;
            }
            pixel.r = addSaturated(pixel.r, colorAdd.r);
            pixel.g = addSaturated(pixel.g, colorAdd.g);
            pixel.b = addSaturated(pixel.b, colorAdd.b);
        }
    }

    void applyPictureRules(Image& image, core::Color colorAdd) {
        for (core::Color& pixel : image.pixels()) {
            if (pixel.r == kTransparentKey.r && pixel.g == kTransparentKey.g && pixel.b == kTransparentKey.b) {
                pixel = {0, 0, 0, 0};
                continue;
            }
            // 8: darkness threshold per channel; dark pixels get blue 8 so they stay visible
            if (pixel.r < 8 && pixel.g < 8 && pixel.b < 8) {
                pixel.b = 8;
            }
            if (pixel.r == 0 && pixel.g == 0 && pixel.b == 8) { // the near-black marker colour (0, 0, 8)
                pixel.a = 255; // opaque
                pixel = to16Bit(pixel);
                continue;
            }
            pixel.r = addSaturated(pixel.r, colorAdd.r);
            pixel.g = addSaturated(pixel.g, colorAdd.g);
            pixel.b = addSaturated(pixel.b, colorAdd.b);
            pixel.a = 255; // opaque
            pixel = to16Bit(pixel);
        }
    }

    void quantizeTo16Bit(Image& image) {
        for (core::Color& pixel : image.pixels()) {
            pixel = to16Bit(pixel);
        }
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
