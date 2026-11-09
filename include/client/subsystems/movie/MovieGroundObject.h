#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_GROUND_OBJECT_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_GROUND_OBJECT_H

namespace client {

    /// @brief A ground object of a movie's map: its kind and draw depth.
    /// @details Original: a TGroundObj of MovieMap as GetDrawSortKey sees it; kinds 1 and 2 lie under the roles, 4 and 5 over them.
    struct MovieGroundObject {
        int kind = 0;
        int depth = 0;
    };

}

#endif
