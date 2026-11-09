#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_FRAME_PICTURE_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_FRAME_PICTURE_H

namespace client {

    /// @brief Frame counting of a strip of frames (original MZ_TPic.pas TPic): current frame, tick and delay.
    struct FramePicture {
        int frame = 0;
        int tick = 0;
        int delay = 4; // default ticks to wait before the next frame
        int count = 0;

        /// @brief Shows the current frame and moves on when the tick reached the delay (original DrawPicAdvance).
        /// @return The frame to draw.
        int advance() {
            if (frame > count) {
                frame = count;
            }
            const int shown = frame;
            if (tick >= delay) {
                tick = 0;
                ++frame;
            } else {
                ++tick;
            }
            return shown;
        }

        /// @brief Shows the current frame and goes back one when the tick reached the delay (original DrawPicStepBack).
        /// @return The frame to draw.
        int stepBack() {
            if (frame < 0) {
                frame = 0;
            }
            const int shown = frame;
            if (tick >= delay) {
                tick = 0;
                --frame;
            } else {
                ++tick;
            }
            return shown;
        }

        /// @brief Whether the last frame is reached.
        /// @return True at or past the last frame.
        bool isLast() const { return frame >= count; }

        /// @brief Whether the first frame is shown.
        /// @return True at frame 0.
        bool isFirst() const { return frame == 0; }

        /// @brief Goes back to the first frame with a cleared tick (original ResetFrame).
        void reset() {
            tick = 0;
            frame = 0;
        }
    };

}

#endif
