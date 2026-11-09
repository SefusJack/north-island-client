#ifndef NORTH_ISLAND_FORMATS_MOVIE_GROUND_H
#define NORTH_ISLAND_FORMATS_MOVIE_GROUND_H

#include "formats/MovieGroundStep.h"

#include <string>
#include <vector>

namespace formats {

    /// @brief The ground of a movie: a JPEG the view scrolls over.
    /// @details Original: Tft_Ground as LoadMovieScript reads it; named as the file says: "JRP" before the name marks a movie picture
    /// of the jrp set (skipped when the memory mode is 2), "Map" and a number the map of that id. Its path of pathCount + 1 points
    /// is walked over the steps pathStart to pathStart + pathCount.
    struct MovieGround {
        /// @brief What the ground's name refers to.
        enum class Kind { Picture, Jrp, Map };

        std::string name;
        int pathStart = 0;
        int pathCount = 0;
        std::vector<MovieGroundStep> path;

        /// @brief Classifies the ground by the first three letters of its name.
        /// @return Picture, Jrp ("JRP" prefix) or Map ("Map" prefix).
        Kind kind() const;

        /// @brief The picture name without the three-letter prefix (the map number as text for a map).
        /// @return The name.
        std::string pictureName() const;
    };

}

#endif
