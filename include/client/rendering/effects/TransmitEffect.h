#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_TRANSMIT_EFFECT_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_TRANSMIT_EFFECT_H

#include "client/rendering/effects/LightEffect.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>

namespace client {

    /// @brief The light a vehicle's trip puts round the own player, with its departure and arrival phases.
    /// @details Original: DZ_Human.pas StartTransmitEffect and UpdateTransmitEffect with the form's FArrivalPhase and FTargetIndex. A column
    /// of light (S10108) for a second and a half in which the player is gone from the 900th millisecond on, then the server is told the
    /// place (69) and the arrival's light (S10123) plays six seconds, the player showing again after the 4800th.
    class TransmitEffect {
        public:
            /// @brief Creates the effect.
            /// @param pictures Picture library the light is looked up in.
            explicit TransmitEffect(
                gfx::PictureLibrary& pictures
            )
                : m_pictures(pictures) {}

            /// @brief Chooses the place and starts the departure.
            /// @details Original: PickTeleportTarget (FTargetIndex; FArrivalPhase 0).
            /// @param place Index of the chosen place.
            /// @param feet Screen position of the player's feet.
            /// @param nowMs Current tick in milliseconds.
            void begin(int place, core::Point feet, std::uint64_t nowMs);

            /// @brief Starts or restarts the light at the player's feet.
            /// @details Original: StartTransmitEffect.
            /// @param feet Screen position of the player's feet.
            /// @param nowMs Current tick in milliseconds.
            void start(core::Point feet, std::uint64_t nowMs);

            /// @brief Starts the arrival's light (only while the effect is on).
            /// @details Original: server action 12/12 while the effect is on (the effect starts, then FArrivalPhase is 1).
            /// @param feet Screen position of the player's feet.
            /// @param nowMs Current tick in milliseconds.
            void arrive(core::Point feet, std::uint64_t nowMs);

            /// @brief Advances the effect for a frame.
            /// @details Original: UpdateTransmitEffect.
            /// @param feet Screen position of the player's feet.
            /// @param nowMs Current tick in milliseconds.
            /// @return True when the server is to be told the place now (69).
            bool update(core::Point feet, std::uint64_t nowMs);

            /// @brief Tells whether the player is not to be drawn.
            /// @details Original: FTransmitEffectOn.
            /// @return True while the player is gone.
            bool hidesPlayer() const { return m_on; }

            /// @brief The chosen place.
            /// @return Index of the place.
            int place() const { return m_place; }

            /// @brief Draws the light right after the player.
            /// @details Original: DrawLight(1).
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current tick in milliseconds.
            void draw(gfx::Canvas& canvas, std::uint64_t nowMs);

        private:
            gfx::PictureLibrary& m_pictures;
            LightEffect m_light;
            bool m_running = false;
            bool m_on = false;
            bool m_arrival = false;
            std::uint64_t m_tickMs = 0;
            std::uint64_t m_durationMs = 0;
            int m_place = 0;
    };

}

#endif
