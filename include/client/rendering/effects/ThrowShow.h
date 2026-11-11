#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_THROW_SHOW_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_THROW_SHOW_H

#include "client/rendering/effects/LightEffect.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief The thrown items of server action 23/117: an item flies in an arc to the clicked point and its impact plays there.
    /// @details Original: DZ_Human.pas HandleThrowFxPacket, HL_NewFireworks.pas THL_EffectManage.StartThrowEffectLight. The item's picture
    /// flies in an arc from 50 pixels over the thrower to the point clicked at 0.36 pixels a millisecond with its shadow on the ground,
    /// then the impact of its kind (S10797, S10241 or S10767) plays there with its sound, and the players under it are told (AddEffectArea).
    class ThrowShow {
        public:
            using SoundPlayer = std::function<void(const std::string&)>;

            using LandingHandler = std::function<void(int kind, core::Point target)>;

            /// @brief Creates the show.
            /// @param pictures Picture library the lights are looked up in.
            /// @param playSound Callback that plays a sound file by name.
            /// @param landed Callback told when a throw came down.
            ThrowShow(
                gfx::PictureLibrary& pictures,
                SoundPlayer playSound,
                LandingHandler landed
            )
                : m_pictures(pictures),
                  m_playSound(std::move(playSound)),
                  m_landed(std::move(landed)) {}

            /// @brief Starts a throw.
            /// @param kind Throw kind 1..3.
            /// @param from Map pixel the item is thrown from.
            /// @param to Map pixel the item comes down at.
            /// @param nowMs Current tick in milliseconds.
            void start(int kind, core::Point from, core::Point to, std::uint64_t nowMs);

            /// @brief Moves and draws every flight and impact; landed ones start their impact.
            /// @details Original: UpdateMapEffectLights (map pixels less the origin).
            /// @param canvas Canvas to draw on.
            /// @param origin Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            void draw(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs);

            /// @brief Tells whether no throw is running.
            /// @return True when there is none.
            bool empty() const { return m_throws.empty(); }

        private:
            /// @brief One throw in flight or playing its impact.
            struct Throw {
                int kind = 0;
                core::Point target;
                LightEffect flight;
                LightEffect impact;
                bool landed = false;
            };

            gfx::PictureLibrary& m_pictures;
            SoundPlayer m_playSound;
            LandingHandler m_landed;
            std::vector<Throw> m_throws;
    };

}

#endif
