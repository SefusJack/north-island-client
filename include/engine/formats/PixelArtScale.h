#ifndef NORTH_ISLAND_FORMATS_PIXEL_ART_SCALE_H
#define NORTH_ISLAND_FORMATS_PIXEL_ART_SCALE_H

#include "formats/Image.h"

#include <optional>
#include <string>

namespace formats {

    /// @brief A pixel-art scaler the qol client can put on the game's pictures, sprites and fonts at a pixel scale above 1x.
    /// @details Scale is AdvMAME's Scale2x / Scale3x (EPX); Xbr is Hyllian's xBR (edges found by weighing colour distances along both diagonals).
    enum class PixelArtFilter { None, Scale, Xbr };

    /// @brief The qol settings' name of a filter.
    /// @param filter The filter.
    /// @return "off", "scale" or "xbr".
    const char* pixelArtFilterName(PixelArtFilter filter);

    /// @brief The filter of a qol settings name.
    /// @param name "off", "scale" or "xbr".
    /// @return The filter, or nothing for an unknown name.
    std::optional<PixelArtFilter> pixelArtFilterNamed(const std::string& name);

    /// @brief Scales a picture up by an integer factor each way.
    /// @details Scale takes factor 2 or 3 and xBR any; None, or Scale at another factor, repeats each pixel.
    /// @param image Source picture.
    /// @param filter Scaler to use.
    /// @param factor Scale factor (below 1 counts as 1).
    /// @return The scaled picture.
    Image scalePixelArt(const Image& image, PixelArtFilter filter, int factor);

}

#endif
