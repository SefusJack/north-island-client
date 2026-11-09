#ifndef NORTH_ISLAND_FORMATS_MOVIE_PICTURE_STEP_H
#define NORTH_ISLAND_FORMATS_MOVIE_PICTURE_STEP_H

namespace formats {

    /// @brief One point of a movie picture's path.
    /// @details Original: Tft_PictureStep, 33 bytes in the .sty file; where its foot goes, the speed (1 to 7), the frame to hold
    /// (0: the strip plays), and the sound that starts there (an id of soundtabel.txt, 0 none; looped or once).
    struct MoviePictureStep {
        int targetX = 0;
        int targetY = 0;
        int moveMode = 0;
        int frameTick = 0;
        int soundNo = 0;
        bool soundLoop = false;
    };

}

#endif
