#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_SNOW_EFFECT_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_SNOW_EFFECT_H

#include "client/rendering/effects/SnowFlake.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureId.h"
#include "gfx/PictureLibrary.h"

#include <array>
#include <cstdint>
#include <random>

namespace client {

    /// @brief The scene snow.
    /// @details Original: TFT_EffectManage.FSnows with UpdateSnowEffect / DrawSnow. Up to a hundred flakes, a new one every half second
    /// from a random place along the top of the screen. Flakes lie on the map, so they move with the camera.
    class SnowEffect {
        public:
            /// @brief Creates the effect and loads the snow pictures.
            /// @param pictures Picture library icon_Snow1..3 are looked up in.
            explicit SnowEffect(
                gfx::PictureLibrary& pictures
            );

            /// @brief Starts a flake when due, moves the falling ones on and drops the landed ones.
            /// @details Original: UpdateSnowEffect.
            /// @param view Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            void update(core::Point view, std::uint64_t nowMs);

            /// @brief Draws the falling flakes.
            /// @details Original: DrawSnow.
            /// @param canvas Canvas to draw on.
            /// @param view Map point at the screen's top-left corner.
            void draw(gfx::Canvas& canvas, core::Point view);

            /// @brief Switches the effect off: flakes start afresh when it is on again.
            void clear();

        private:
            static constexpr int kFlakes = 100; // most flakes at once

            /// @brief Gives a flake its start place, target and look.
            /// @details Original: ResetSnow.
            /// @param flake Flake to start.
            /// @param view Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            void reset(SnowFlake& flake, core::Point view, std::uint64_t nowMs);

            /// @brief Moves a flake towards its target by the time since its last frame change and steps its frame.
            /// @details Original: MoveSnow (StepSnow).
            /// @param flake Flake to move.
            /// @param nowMs Current tick in milliseconds.
            static void move(SnowFlake& flake, std::uint64_t nowMs);

            /// @brief Advances the sideways shake of a flake.
            /// @details Original: AnimateSnow.
            /// @param flake Flake to shake.
            static void shake(SnowFlake& flake);

            /// @brief Draws a random number.
            /// @param count Number of values.
            /// @return A value in 0..count-1.
            int random(int count);

            gfx::PictureLibrary& m_library;
            std::array<gfx::PictureId, 3> m_pictures; // icon_Snow1..3
            std::array<SnowFlake, kFlakes> m_flakes;
            std::uint64_t m_spawnMs = 0;
            bool m_started = false;
            std::mt19937 m_random{std::random_device{}()};
    };

}

#endif
