#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_FISH_TANK_FISH_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_FISH_TANK_FISH_H

#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureId.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief A fish of the fish tank form: a picture of 4 frames by 2 facings that swims between random places.
    /// @details Original: MK_Menu.pas TFish.
    struct TankFish {
            using Random = std::function<int(int)>;

            gfx::PictureId picture;
            int posX = 0;
            int posY = 0;
            int destX = 0;
            int destY = 0;
            int facing = 0;
            int frame = 0;
            int frameWidth = 0;
            int frameHeight = 0;
            int stepX = 0;
            int stepY = 0;
            int maxX = 0;
            int maxY = 0;
            std::uint64_t frameMs = 0;
            std::uint64_t motionMs = 0;
            int speedBonusX = 0;
            int speedBonusY = 0;
            bool active = false;
            int minY = 0;
            bool inTank = false;
            int swimDist = 0;
            bool surfacing = false;

            /// @brief Sets the fish's picture (not valid: no fish), its size, and a first place and destination at random.
            /// @details Original: SetFishPic.
            /// @param newPicture Picture of the kind of fish.
            /// @param pictureSize Size of the whole picture.
            /// @param random Delphi-style Random function.
            void setPicture(gfx::PictureId newPicture, core::Point pictureSize, const Random& random);

            /// @brief Removes the fish.
            /// @details Original: ClearFish.
            void clear();

            /// @brief Moves the fish one frame towards its destination.
            /// @details Original: UpdateFishMotion.
            /// @param random Delphi-style Random function.
            /// @param nowMs Current time in milliseconds.
            void update(const Random& random, std::uint64_t nowMs);

            /// @brief Draws the fish at the form's origin; the form's width keeps it from swimming out of the right edge.
            /// @details Original: DrawFish.
            /// @param canvas Target canvas.
            /// @param origin Top-left of the form.
            /// @param formWidth Width of the form.
            /// @param nowMs Current time in milliseconds.
            void draw(gfx::Canvas& canvas, core::Point origin, int formWidth, std::uint64_t nowMs);

        private:
            /// @brief Sinks the in-tank fish a pixel a frame for a second, or to the tank's floor.
            /// @details Original: UpdateFishSurfacing.
            /// @param nowMs Current time in milliseconds.
            void updateSurfacing(std::uint64_t nowMs);

            /// @brief Works out the pixel step across and down towards the destination.
            /// @details Original: CalcFishStep.
            void calcStep();
    };

}

#endif
