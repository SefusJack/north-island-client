#ifndef NORTH_ISLAND_FORMATS_MOVIE_ROLE_STEP_H
#define NORTH_ISLAND_FORMATS_MOVIE_ROLE_STEP_H

namespace formats {

    /// @brief One point of a movie role's path.
    /// @details Original: Tft_RolePathElem, 45 bytes in the .sty file; fixed direction, place, speed, the action (255: keep the one before),
    /// the frame, the trail mode (1 to 3 a trail, 4 to 6 a drawing mode) and the sound that starts there.
    struct MovieRoleStep {
        int fixedDir = 0;
        int x = 0;
        int y = 0;
        int moveSpeed = 0;
        int action = 0;
        int frame = 0;
        int pathMode = 0;
        int soundNo = 0;
        bool soundLoop = false;
    };

}

#endif
