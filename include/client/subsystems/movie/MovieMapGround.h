#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_MAP_GROUND_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_MAP_GROUND_H

#include "client/subsystems/movie/MovieGroundObject.h"
#include "core/Size.h"

#include <vector>

namespace client {

    /// @brief The map a movie takes for its ground: its size and ground objects.
    /// @details Original: LoadMovieScript's "Map10001" via LoadMapFile, or the game's own map when the player is on it.
    struct MovieMapGround {
        core::Size size;
        std::vector<MovieGroundObject> objects;
    };

}

#endif
