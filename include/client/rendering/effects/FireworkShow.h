#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_FIREWORK_SHOW_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_FIREWORK_SHOW_H

#include "client/rendering/effects/LightEffect.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief The firework show of server action 23/148 (a rocket that rises, then a burst by kind and variant).
    /// @details Original: DZ_Human.pas StartEffectS51016 and UpdateEffectS51016; rocket lights s51016L and S50616L (six frames of 280 ms)
    /// rise from 50 pixels over the player's feet, the burst plays where the rocket ended; the whole is cut off after 2.8 seconds.
    class FireworkShow {
        public:
            using SoundPlayer = std::function<void(const std::string&)>;

            /// @brief Creates the show.
            /// @param pictures Picture library the light strips are looked up in.
            /// @param playSound Callback that plays a sound file by name.
            FireworkShow(
                gfx::PictureLibrary& pictures,
                SoundPlayer playSound
            )
                : m_pictures(pictures),
                  m_playSound(std::move(playSound)) {}

            /// @brief Starts the show at the player's map position.
            /// @details Original: StartEffectS51016.
            /// @param kind Firework kind 1..4.
            /// @param variant Variant of the kind (picks the burst picture of kind 1).
            /// @param at Map position of the player.
            /// @param nowMs Current tick in milliseconds.
            void start(int kind, int variant, core::Point at, std::uint64_t nowMs);

            /// @brief Moves and draws the show.
            /// @details Original: UpdateEffectS51016.
            /// @param canvas Canvas to draw on.
            /// @param origin Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            /// @return False once the show is over (the item may be used again).
            bool draw(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs);

        private:
            /// @brief Starts a light strip played once at a map point, drawn added to the screen.
            /// @param light Light to start.
            /// @param name Picture name of the strip.
            /// @param frameMs Milliseconds per frame.
            /// @param frames Number of frames in the picture.
            /// @param offsetY Pixels the strip is drawn down from the point.
            /// @param nowMs Current tick in milliseconds.
            /// @param at Map point the strip is played at.
            void setup(
                LightEffect& light,
                const char* name,
                std::uint64_t frameMs,
                int frames,
                int offsetY,
                std::uint64_t nowMs,
                core::Point at
            );

            gfx::PictureLibrary& m_pictures;
            SoundPlayer m_playSound;
            LightEffect m_rocket;
            LightEffect m_rocketGlow;
            LightEffect m_burst;
            LightEffect m_second;
            int m_kind = 0;
            int m_variant = 0;
            int m_burstOffset = 0;
            int m_secondStart = 1;
            bool m_bursting = false;
            bool m_running = false;
            double m_rise = 0;
            core::Point m_anchor;
            std::uint64_t m_startMs = 0;
            std::uint64_t m_lastMs = 0;
    };

}

#endif
