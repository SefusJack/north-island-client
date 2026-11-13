#ifndef NORTH_ISLAND_FORMATS_GAME_PIXELS_H
#define NORTH_ISLAND_FORMATS_GAME_PIXELS_H

#include "core/Color.h"
#include "formats/Image.h"

#include <cstdint>
#include <vector>

namespace formats {

    constexpr core::Color kTransparentKey{0, 255, 0, 255}; // pure green (R 0, G 255, B 0), opaque: the colour key treated as transparent

    /// @brief Applies the picture cache's colour rules to a bitmap (transparent key, near-black fix, tint, RGB565).
    /// @details Original: the game's 16-bit picture cache load (AddPicFromStreamVcl).
    /// @param image Bitmap changed in place.
    /// @param colorAdd Tint added to red, green and blue, saturating at 255.
    void applyPictureRules(Image& image, core::Color colorAdd = {0, 0, 0, 0});

    /// @brief Reduces every pixel to RGB565 precision (JPEG backgrounds drawn onto the 16-bit screen).
    /// @param image Bitmap changed in place.
    void quantizeTo16Bit(Image& image);

    /// @brief Adds a colour to a run of pixels, as addColor does to a whole picture.
    /// @param pixels The first pixel.
    /// @param count How many pixels.
    /// @param colorAdd The colour to add.
    void addColor(core::Color* pixels, std::size_t count, core::Color colorAdd);

    /// @brief Reduces a run of pixels to RGB565 precision (part of a picture).
    /// @param colors The first pixel.
    /// @param count How many pixels.
    void quantizeTo16Bit(core::Color* colors, std::size_t count);

    /// @brief Adds a colour to every pixel, saturating at 255 (a JPEG background in a tinted map).
    /// @details Original: TIJL_JpgToBmp.DecodeRegionTo16; pixels whose channels all stay under 4 are left as they are.
    /// @param image Bitmap changed in place.
    /// @param colorAdd Colour added to red, green and blue.
    void addColor(Image& image, core::Color colorAdd);

    /// @brief Lays a colour over every pixel with the 16-bit screen's alpha-fill arithmetic (the dimmed world).
    /// @details Original: ro_Clipper_Alpha_Fill (rodraw2) on the 16-bit screen; arithmetic on the RGB565 fields floors where an 8-bit blend rounds.
    /// @param image Bitmap changed in place.
    /// @param fill Fill colour; its alpha is the strength.
    void alphaFill16(Image& image, core::Color fill);

    /// @brief Blends a colour over every pixel like the 16-bit screen's rectangle alpha fill.
    /// @details Original: TDirectDrawSurface.FillRectAlpha (DelphiX dxrFillRectColorBlend, DXR_BLEND_SRCALPHA1_ADD_INVSRCALPHA2);
    /// red and green are (fill * alpha + lane * (255 - alpha)) >> 8, blue is lane + ((fill - lane) * alpha >> 8); floors where an 8-bit blend rounds.
    /// @param image Bitmap changed in place.
    /// @param fill Fill colour; its alpha is the strength.
    void fillAlpha16(Image& image, core::Color fill);

    /// @brief One bit per pixel, set where the pixel is not fully transparent: row after row, 64 pixels per word (bit 0 first).
    /// @param image The picture.
    /// @return The bits.
    std::vector<std::uint64_t> opaqueBits(const Image& image);

}

#endif
