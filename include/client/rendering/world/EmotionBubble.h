#ifndef NORTH_ISLAND_CLIENT_RENDERING_WORLD_EMOTION_BUBBLE_H
#define NORTH_ISLAND_CLIENT_RENDERING_WORLD_EMOTION_BUBBLE_H

#include "core/Point.h"
#include "gfx/Canvas.h"

#include <cstdint>

namespace client {

    /// @brief An emotion over a character's head: the strip E<id> plays a frame every 200 ms, four times through, beside the head.
    /// @details Original: ShowEmotion / DrawEmotion.
    class EmotionBubble {
        public:
            /// @brief Starts an emotion.
            /// @details Original: ShowEmotion.
            /// @param emotionId Emotion id (the picture strip E<id>).
            /// @param nowMs Current time in milliseconds.
            void show(int emotionId, std::uint64_t nowMs);

            /// @brief Draws the current frame and advances the animation.
            /// @details Original: DrawEmotion.
            /// @param canvas Target canvas.
            /// @param feet Position of the character's feet on the screen.
            /// @param height How tall the character is drawn (FSpriteHeight).
            /// @param nowMs Current time in milliseconds.
            void draw(gfx::Canvas& canvas, core::Point feet, int height, std::uint64_t nowMs);

        private:
            int m_emotionId = 0;
            int m_frame = 0;
            int m_loops = 0;
            std::uint64_t m_frameMs = 0;
    };

}

#endif
