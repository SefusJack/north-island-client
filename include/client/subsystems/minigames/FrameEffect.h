#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_FRAME_EFFECT_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_FRAME_EFFECT_H

#include "core/Point.h"
#include "core/Size.h"
#include "gfx/Canvas.h"
#include "gfx/PictureId.h"

namespace client {

    /// @brief A light running round a frame (original CJ_MiniGame.pas TFrameEffect, mode 1).
    class FrameEffect {
        public:
            /// @brief Starts the effect (original InitFrameEffect for mode 1 with a plain blit).
            /// @param picture Frame picture to cut the pieces from.
            /// @param topLeft Top left of the frame.
            /// @param size Size of the frame.
            /// @param segmentLength Length of the short travelling piece.
            /// @param thickness Thickness of the thick rim piece.
            void start(
                gfx::PictureId picture,
                core::Point topLeft,
                core::Size size,
                int segmentLength,
                int thickness
            );

            /// @brief Draws the two travelling pieces and takes a step on the beat's frame (original DrawFrameEffect).
            /// @param canvas Canvas to draw on.
            void draw(gfx::Canvas& canvas);

            /// @brief Whether the effect has a picture to run.
            /// @return True when started.
            bool active() const { return m_picture.isValid(); }

        private:
            gfx::PictureId m_picture;
            core::Point m_topLeft;
            core::Point m_bottomRight;
            int m_posH = 0;
            int m_posV = 0;
            int m_shownH = 0;
            int m_shownV = 0;
            int m_segment = 0;
            int m_thickness = 0;
            int m_width = 0;
            int m_height = 0;
    };

}

#endif
