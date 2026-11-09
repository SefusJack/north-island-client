#ifndef NORTH_ISLAND_FORMATS_MOVIE_GROUND_STEP_H
#define NORTH_ISLAND_FORMATS_MOVIE_GROUND_STEP_H

namespace formats {

    /// @brief One point of a movie ground's path.
    /// @details Original: Tft_GroundStep, 33 bytes in the .sty file; where the view's centre goes and the scroll speed (1 to 7).
    struct MovieGroundStep {
        int targetX = 0;
        int targetY = 0;
        int scrollMode = 0;
    };

}

#endif
