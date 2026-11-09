#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_WISH_EFFECTS_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_WISH_EFFECTS_H

#include "client/rendering/effects/LightEffect.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <utility>

namespace client {

    /// @brief The lights the Bliss Lamp and the Passion Bouquet put on players.
    /// @details Original: DZ_Human.pas StartBlessEffect and UpdateBlessEffect: a balloon (S40015, four frames) that rises from the player's
    /// side for six seconds, and the hearts over the target of a bouquet (StartEffectS40011 and UpdateEffectS40011: S40011, nine frames,
    /// played again for ten seconds).
    class WishEffects {
        public:
            /// @brief Creates the effects.
            /// @param pictures Picture library the lights are looked up in.
            /// @param playSound Callback that plays a sound file by name.
            WishEffects(
                gfx::PictureLibrary& pictures,
                std::function<void(const std::string&)> playSound
            )
                : m_pictures(pictures),
                  m_playSound(std::move(playSound)) {}

            /// @brief Starts the balloon of a player.
            /// @details Original: StartBlessEffect.
            /// @param playerId Player the balloon belongs to.
            /// @param feet Map pixel the player stands at.
            /// @param direction Way the player faces (0 to 7) when his action turns him; nullopt starts it at his left.
            /// @param nowMs Current tick in milliseconds.
            void startBless(std::int32_t playerId, core::Point feet, std::optional<int> direction, std::uint64_t nowMs);

            /// @brief Starts the hearts over a player.
            /// @details Original: StartEffectS40011.
            /// @param playerId Target player of the bouquet.
            /// @param nowMs Current tick in milliseconds.
            void startHearts(std::int32_t playerId, std::uint64_t nowMs);

            /// @brief Draws the lights of a frame, after the people.
            /// @details Original: UpdateBlessEffect and UpdateEffectS40011.
            /// @param canvas Canvas to draw on.
            /// @param origin Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            /// @param positionOf Gives where a player is now (map pixels).
            /// @return True when a balloon ended (Player.FBlessDone := 1).
            bool draw(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs, const std::function<std::optional<core::Point>(std::int32_t)>& positionOf);

        private:
            /// @brief The balloon of one player.
            struct Balloon {
                LightEffect light;
                core::Point at;
                std::uint64_t startMs = 0;
            };

            /// @brief The hearts over one player.
            struct Hearts {
                LightEffect light;
                std::uint64_t startMs = 0;
            };

            /// @brief Starts the hearts strip once.
            /// @param hearts Hearts to start.
            /// @param nowMs Current tick in milliseconds.
            void startHeartsLight(Hearts& hearts, std::uint64_t nowMs);

            gfx::PictureLibrary& m_pictures;
            std::function<void(const std::string&)> m_playSound;
            std::map<std::int32_t, Balloon> m_balloons;
            std::map<std::int32_t, Hearts> m_hearts;
    };

}

#endif
