#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_TIMED_LIGHT_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_TIMED_LIGHT_H

#include "client/rendering/effects/LightEffect.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>

namespace client {

    /// @brief The timed light S10233 round the own player.
    /// @details Original: StartEffectS10233, UpdateEffectS10233. Sixteen frames played over and over for a minute, each use of server
    /// action 23/38 adding another minute.
    class TimedLight {
        public:
            /// @brief Creates the effect.
            /// @param pictures Picture library the light is looked up in.
            explicit TimedLight(
                gfx::PictureLibrary& pictures
            )
                : m_pictures(pictures) {}

            /// @brief Starts the light, or adds a minute to it when it is on.
            /// @details Original: StartEffectS10233, server action 23/38.
            /// @param feet Screen position of the player's feet.
            /// @param nowMs Current tick in milliseconds.
            void start(core::Point feet, std::uint64_t nowMs);

            /// @brief Draws the light on the player's feet and ends it when its time is out.
            /// @details Original: UpdateEffectS10233.
            /// @param canvas Canvas to draw on.
            /// @param feet Screen position of the player's feet.
            /// @param nowMs Current tick in milliseconds.
            void draw(gfx::Canvas& canvas, core::Point feet, std::uint64_t nowMs);

        private:
            /// @brief Plays the strip once at the player's feet.
            /// @param feet Screen position of the player's feet.
            /// @param nowMs Current tick in milliseconds.
            void play(core::Point feet, std::uint64_t nowMs);

            gfx::PictureLibrary& m_pictures;
            LightEffect m_light;
            bool m_on = false;
            std::uint64_t m_startMs = 0;
            std::uint64_t m_durationMs = 0;
    };

}

#endif
