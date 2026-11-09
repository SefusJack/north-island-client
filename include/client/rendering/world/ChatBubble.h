#ifndef NORTH_ISLAND_CLIENT_RENDERING_WORLD_CHAT_BUBBLE_H
#define NORTH_ISLAND_CLIENT_RENDERING_WORLD_CHAT_BUBBLE_H

#include "core/Color.h"
#include "core/Point.h"
#include "core/Size.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>
#include <string>

namespace client {

    /// @brief What a character last said, over its head for five seconds.
    /// @details Original: SetChatBubble / DrawChatBubble. The TalkForm1 balloon: one row up to ten characters wide, wider up to twenty, then rows of twenty (sixty characters at most), the emote codes drawn as their animated icons.
    class ChatBubble {
        public:
            /// @brief Sets the words shown: the old words go; a speaker not fighting is not heard when the new ones hold a refused word.
            /// @details Original: SetChatBubble.
            /// @param text The words.
            /// @param nowMs Current time in milliseconds.
            /// @param speakerFights True when the speaker is fighting (no word filter).
            void show(const std::string& text, std::uint64_t nowMs, bool speakerFights = false);

            /// @brief Draws the balloon and the words while they are shown.
            /// @details Original: DrawChatBubble.
            /// @param canvas Target canvas.
            /// @param pictures Picture library (TalkForm1 and the emote icons).
            /// @param feet Position of the character's feet on the screen.
            /// @param body Size of the body's frame (GetBodySpriteWidth / Height).
            /// @param nowMs Current time in milliseconds.
            void draw(
                gfx::Canvas& canvas,
                gfx::PictureLibrary& pictures,
                core::Point feet,
                core::Size body,
                std::uint64_t nowMs
            );

        private:
            std::string m_text;
            std::uint64_t m_sinceMs = 0;
    };

}

#endif
