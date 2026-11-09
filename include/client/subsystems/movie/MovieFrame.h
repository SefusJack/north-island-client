#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_FRAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_FRAME_H

#include "core/Color.h"
#include "core/Point.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief What DrawMovie puts on the screen in a frame, in draw order, for the movie layer to paint.
    struct MovieFrame {
        /// @brief A picture's frame: a strip of rows x cols frames, the frame numbered from 1 drawn with its foot at the place.
        /// @details Original: DrawPicture; alpha 255 draws plainly, others (and every one while the screen fades) use the LightAlpha blit.
        struct Picture {
            std::string name;
            core::Point foot;
            int frame = 1;
            int rows = 1;
            int cols = 1;
            int alpha = 255; // fully opaque
            bool light = false;
        };

        /// @brief A role to draw: who it is, where, which action slot and frame, size, flip and afterimage data.
        /// @details Original: DrawRole; surface is an Npc.dat id, 65535 the player, 99901 to 99917 the team and the pets; scale is DrawScale over 10;
        /// flipped is path mode 4; alpha and drawMode (1 plain, 2 blended, 3 ghost, 4 next ghost) are for afterimages of path modes 1 to 3.
        struct Role {
            int surface = 0;
            core::Point foot;
            int slot = 0;
            int frame = 0;
            int scale = 10; // normal size (FHeightScale is 10)
            bool flipped = false;
            bool trail = false;
            int alpha = 255; // fully opaque
            int drawMode = 1;
            bool shadow = false;
            int action = 0;
            bool face = false;
            int faceFrame = 0;
            bool faceFlip = false;
        };

        /// @brief One entry of the draw order: a role, a picture, or a ground object of the map.
        /// @details Original: SortDrawList sorts roles, pictures and ground objects together by their feet.
        struct Drawn {
            bool role = false;
            bool object = false;
            std::size_t index = 0;
        };

        bool groundShown = true;
        bool mapGround = false;
        std::string groundName;
        core::Point groundOrigin;
        std::optional<int> groundDarkening;
        std::vector<Picture> pictures;
        std::vector<Role> roles;
        std::vector<Drawn> order;
        int efficacyMode = 0;
        std::optional<core::Color> fill;
    };

}

#endif
