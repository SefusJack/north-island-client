#ifndef NORTH_ISLAND_FORMATS_MOVIE_PICTURE_H
#define NORTH_ISLAND_FORMATS_MOVIE_PICTURE_H

#include "formats/MoviePictureStep.h"

#include <string>
#include <vector>

namespace formats {

    /// @brief A moving picture of a movie.
    /// @details Original: Tft_Picture as LoadMovieScript reads it; a strip of frameRows x frameCols frames of a named picture, drawn by alpha
    /// (255: opaque, else the LightAlpha blit), a frame every interval ms, played `loops` times (0: not drawn), over the steps pathStart
    /// to pathStart + pathCount along its path (pathCount + 1 points).
    struct MoviePicture {
        std::string name;
        int pathStart = 0;
        int pathCount = 0;
        int transparentColor = 0;
        int alpha = 255; // 255 = opaque
        int frameRows = 1;
        int frameCols = 1;
        int interval = 0;
        int loops = 0;
        std::vector<MoviePictureStep> path;
        int sortOffsetY = 0;
    };

}

#endif
