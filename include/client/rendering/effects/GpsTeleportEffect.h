#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_GPS_TELEPORT_EFFECT_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_GPS_TELEPORT_EFFECT_H

#include "client/rendering/effects/LightEffect.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>

namespace client {

    /// @brief The light of a GPS teleport round the own player, with its departure and arrival phases.
    /// @details Original: DZ_Human.pas StartGpsTeleportEffect and UpdateGpsTeleportEffect with the form's FArrivalPhase. A column of light
    /// (picture S10882) for 1.5 seconds in which the player is gone from the 900th millisecond on, then the server is told to teleport
    /// (74/2) and the arrival's light plays until the sixth second, the player showing again after the 4800th.
    class GpsTeleportEffect {
        public:
            /// @brief Creates the effect.
            /// @param pictures Picture library the light is looked up in.
            explicit GpsTeleportEffect(
                gfx::PictureLibrary& pictures
            )
                : m_pictures(pictures) {}

            /// @brief Starts the departure.
            /// @details Original: StartGpsTeleportEffect(player, 0, 0).
            /// @param feet Position of the player's feet.
            /// @param nowMs Current tick in milliseconds.
            void begin(core::Point feet, std::uint64_t nowMs);

            /// @brief Starts the arrival's light on the new map (only while the effect is on).
            /// @details Original: 12/12 while the effect is on: StartGpsTeleportEffect(player, 1, 0).
            /// @param feet Position of the player's feet.
            /// @param nowMs Current tick in milliseconds.
            void arrive(core::Point feet, std::uint64_t nowMs);

            /// @brief Advances the effect for a frame.
            /// @details Original: UpdateGpsTeleportEffect.
            /// @param feet Position of the player's feet.
            /// @param nowMs Current tick in milliseconds.
            /// @return True when the server is to be told to teleport now (74/2).
            bool update(core::Point feet, std::uint64_t nowMs);

            /// @brief Tells whether the player is not to be drawn.
            /// @details Original: FGpsTeleportEffectOn.
            /// @return True while the player is gone.
            bool hidesPlayer() const { return m_on; }

            /// @brief Draws the light right after the player.
            /// @details Original: DrawLight(1).
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current tick in milliseconds.
            void draw(gfx::Canvas& canvas, std::uint64_t nowMs);

        private:
            /// @brief Starts or restarts the light at the player's feet.
            /// @details Original: StartGpsTeleportEffect.
            /// @param feet Position of the player's feet.
            /// @param nowMs Current tick in milliseconds.
            void start(core::Point feet, std::uint64_t nowMs);

            gfx::PictureLibrary& m_pictures;
            LightEffect m_light;
            bool m_running = false;
            bool m_on = false;
            bool m_arrival = false;
            std::uint64_t m_tickMs = 0;
            std::uint64_t m_durationMs = 0;
    };

}

#endif
