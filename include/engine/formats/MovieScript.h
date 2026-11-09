#ifndef NORTH_ISLAND_FORMATS_MOVIE_SCRIPT_H
#define NORTH_ISLAND_FORMATS_MOVIE_SCRIPT_H

#include "core/Bytes.h"
#include "formats/MovieGround.h"
#include "formats/MoviePicture.h"
#include "formats/MovieRole.h"
#include "formats/MovieStepEffect.h"
#include "formats/MovieTalk.h"

#include <optional>
#include <vector>

namespace formats {

    /// @brief A movie script of the sty folder.
    /// @details Original: FT_Movie_Object.pas LoadMovieScript. A scale byte, the counts of main roles (0 or 1), roles, pictures, grounds and talks
    /// and the step count; then the main role, the roles, the pictures and the ground, the minimum time of each step in ms (the crunode table,
    /// steps + 1 values), each step's screen effect (steps + 1 records) and the talks (one more than the count). Integers are 32 bits
    /// little-endian; names are Pascal strings in 14 bytes. Every script of the game ends 8 bytes after its last table, which the loader never reads.
    /// A script that is cut short, or whose counts would overrun the original's arrays, is refused.
    struct MovieScript {
        static constexpr int kMostRoles = 20; // size of the original's role array
        static constexpr int kMostPictures = 30; // size of the original's picture array
        static constexpr int kMostTalks = 100; // size of the original's talk array
        static constexpr int kMostPathPoints = 256; // size of the original's path point array

        /// @brief Parses a .sty movie script.
        /// @param data Raw file bytes.
        /// @return The script, or nothing when it is cut short or its counts are too large.
        static std::optional<MovieScript> parse(const core::Bytes& data);

        std::optional<MovieRole> mainRole;
        std::vector<MovieRole> roles;
        std::vector<MoviePicture> pictures;
        MovieGround ground;
        int stepCount = 0;
        std::vector<int> stepMillis;
        std::vector<MovieStepEffect> effects;
        std::vector<MovieTalk> talks;
    };

}

#endif
