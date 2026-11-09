#ifndef NORTH_ISLAND_FORMATS_MOVIE_ROLE_H
#define NORTH_ISLAND_FORMATS_MOVIE_ROLE_H

#include "formats/MovieRoleStep.h"

#include <vector>

namespace formats {

    /// @brief A role of a movie.
    /// @details Original: Tft_Role as LoadMovieScript reads it; who it is (an Npc.dat id, or 99901 to 99917 for the player, the team and the pets),
    /// the steps of the movie it moves in (pathStart to pathStart + pathCount) and its path (pathCount + 1 points).
    struct MovieRole {
        int surface = 0;
        int pathStart = 0;
        int pathCount = 0;
        int direction = 0;
        std::vector<MovieRoleStep> path;
        int sortOffsetY = 0;
    };

}

#endif
