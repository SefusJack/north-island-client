#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_FISH_TANK_BUBBLE_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_FISH_TANK_BUBBLE_H

#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureId.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief A bubble rising in the fish tank form: a strip of 4 frames that starts at a fixed place now and then.
    /// @details Original: MK_Menu.pas TBubble.
    struct TankBubble {
        using Random = std::function<int(int)>;

        gfx::PictureId picture;
        int posX = 0;
        int posY = 0;
        int topY = 0;
        int frame = 0;
        int frameWidth = 0;
        int frameHeight = 0;
        bool rising = false;
        int riseSpeed = 2;
        int startX = 0;
        int startY = 0;
        std::uint64_t frameMs = 0;

        /// @brief Sets the strip picture; its size gives the frame.
        /// @details Original: TBubble.Create.
        /// @param newPicture Picture strip of 4 frames.
        /// @param pictureSize Size of the whole strip.
        void setPicture(gfx::PictureId newPicture, core::Point pictureSize);

        /// @brief Rises two pixels a frame to the top, then has a one in fifty chance a frame to start again.
        /// @details Original: UpdateBubble.
        /// @param random Delphi-style Random function.
        void update(const Random& random);

        /// @brief Draws the bubble while it rises and steps its animation frame.
        /// @details Original: DrawBubble.
        /// @param canvas Target canvas.
        /// @param origin Top-left of the form.
        /// @param nowMs Current time in milliseconds.
        void draw(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs);
    };

}

#endif
