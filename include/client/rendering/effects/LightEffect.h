#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_LIGHT_EFFECT_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_LIGHT_EFFECT_H

#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureId.h"

#include <cstdint>

namespace client {

    /// @brief A strip of picture frames played at a point, optionally flying to a target.
    /// @details Original: TLight (StartLightEffect, DrawLight, MoveLightTowardTarget). Frames firstFrame..lastFrame (1-based) of a picture
    /// split into frameCount, drawn centred on x with their bottom on y. Staying it loops (or stops on its last frame); played once it ends
    /// after its last frame; flying it moves towards a target at a speed in pixels per millisecond and ends there; an arc one rises over
    /// its way with its shadow on the ground. With free frames a flying one is drawn between its last two places.
    class LightEffect {
        public:
            /// @brief How the light moves.
            enum class Motion { Stay, Once, ToTarget, Arc };

            /// @brief Parameters of a light strip.
            struct Setup {
                gfx::PictureId picture;
                std::uint64_t frameMs = 200; // ms each frame is shown
                int firstFrame = 1;
                int lastFrame = 1;
                int frameCount = 1;
                Motion motion = Motion::Stay;
                bool loop = true;
                core::Point target;
                double speed = 0;
                int alpha = 255; // 255 is fully opaque
                int offsetY = 0;
            };

            /// @brief Starts the light.
            /// @details Original: StartLightEffect (and StartArcMove for the arc distance).
            /// @param setup Strip parameters.
            /// @param from Start position.
            /// @param nowMs Current tick in milliseconds.
            void start(const Setup& setup, core::Point from, std::uint64_t nowMs);

            /// @brief Ends the light.
            void stop() { m_active = false; }

            /// @brief Tells whether the light is still playing.
            /// @return True while active.
            bool isActive() const { return m_active; }

            /// @brief How far into its frames the light is.
            /// @details Original: FFrameIndex.
            /// @return Zero-based frame index (0 the first).
            int frameIndex() const { return m_frameIndex; }

            /// @brief Tells whether a flying light has reached its target.
            /// @return True at the target.
            bool atTarget() const { return m_atTarget; }

            /// @brief Moves the light to a position.
            /// @param at New position.
            void setPosition(core::Point at) { m_position = at; }

            /// @brief Moves a flying light on by the time since its last move.
            /// @details Original: UpdateLight.
            /// @param nowMs Current tick in milliseconds.
            void update(std::uint64_t nowMs);

            /// @brief Draws the current frame, then steps to the next one when due.
            /// @details Original: DrawLight.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current tick in milliseconds.
            /// @param origin What is taken off the position to get the screen's (a map effect whose position is in map pixels).
            void draw(gfx::Canvas& canvas, std::uint64_t nowMs, core::Point origin = {});

            /// @brief Current position of the light.
            /// @return The position.
            core::Point position() const { return m_position; }

        private:
            Setup m_setup;
            core::Point m_position;
            core::Point m_previous;
            bool m_active = false;
            bool m_atTarget = false;
            int m_frameIndex = 0;
            std::uint64_t m_frameMs = 0;
            std::uint64_t m_moveMs = 0;
            int m_arcDistance = 0;
    };

}

#endif
