#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_FISH_TANK_FOOD_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_FISH_TANK_FOOD_H

#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureId.h"

#include <cstdint>

namespace client {

    /// @brief A grain of fish food sinking in the fish tank form: a strip of 4 frames.
    /// @details Original: MK_Menu.pas TFood.
    struct TankFood {
        gfx::PictureId picture;
        int posX = 0;
        int posY = 0;
        int frame = 0;
        int frameWidth = 0;
        int frameHeight = 0;
        bool active = false;
        int fallSpeed = 0;
        std::uint64_t frameMs = 0;

        /// @brief Sets the strip picture; its size gives the frame.
        /// @details Original: TFood.Create.
        /// @param newPicture Picture strip of 4 frames.
        /// @param pictureSize Size of the whole strip.
        void setPicture(gfx::PictureId newPicture, core::Point pictureSize);

        /// @brief Draws the grain and steps its animation frame; past the form's right or bottom edge the grain is gone.
        /// @details Original: DrawFood.
        /// @param canvas Target canvas.
        /// @param origin Top-left of the form.
        /// @param formSize The form's width and height.
        /// @param nowMs Current time in milliseconds.
        void draw(gfx::Canvas& canvas, core::Point origin, core::Point formSize, std::uint64_t nowMs);
    };

}

#endif
