#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_PLAYER_EFFECT_LIGHTS_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_PLAYER_EFFECT_LIGHTS_H

#include "client/rendering/effects/LightEffect.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <array>
#include <cstdint>
#include <map>
#include <random>
#include <vector>

namespace client {

    /// @brief The sparkling lights round a player that server action 23/55 puts on.
    /// @details Original: TBasePlayer.FEffectLights, StartEffectLight, DrawEffectLight. Up to three, each a sparkle (L10251) played four
    /// times from a random frame near the place it was put, then the picture of its kind (frame n of L10252) for good. They stand on the
    /// map, not on the player.
    class PlayerEffectLights {
        public:
            /// @brief Creates the lights holder.
            /// @param pictures Picture library the light strips are looked up in.
            /// @param random Random generator shared with the caller.
            PlayerEffectLights(
                gfx::PictureLibrary& pictures,
                std::mt19937& random
            )
                : m_pictures(pictures),
                  m_random(random) {}

            /// @brief Frees the player's lights and puts the given kinds on his places 1..3.
            /// @details Original: RecvPlayerEffectLights (a kind of 249 to 254 ends the list).
            /// @param playerId Player the lights belong to.
            /// @param feet Map position of the player's feet.
            /// @param kinds Light kinds for places 1..3.
            /// @param nowMs Current tick in milliseconds.
            void set(std::int32_t playerId, core::Point feet, const std::vector<int>& kinds, std::uint64_t nowMs);

            /// @brief Drops the lights of a player.
            /// @param playerId Player to forget.
            void remove(std::int32_t playerId) { m_players.erase(playerId); }

            /// @brief Drops the lights of all players.
            void clear() { m_players.clear(); }

            /// @brief Draws the three lights of a player.
            /// @details Original: DrawEffectLight for the three places.
            /// @param canvas Canvas to draw on.
            /// @param playerId Player whose lights are drawn.
            /// @param origin Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            void draw(gfx::Canvas& canvas, std::int32_t playerId, core::Point origin, std::uint64_t nowMs);

        private:
            /// @brief One of the three lights of a player.
            struct Slot {
                LightEffect light;
                bool used = false;
                int stage = 0;
                int kind = 0;
                core::Point place;
            };

            /// @brief Draws a random number.
            /// @param count Number of values.
            /// @return A value in 0..count-1.
            int random(int count);

            /// @brief Starts a light on a random spot near the feet with the first sparkle.
            /// @details Original: StartEffectLight (a spot 20..59 below and up to 50 either side of the feet, the first four frames of the sparkle).
            /// @param slot Light to start.
            /// @param slotNo Place number 1..3.
            /// @param feet Map position of the player's feet.
            /// @param kind Light kind shown at the end.
            /// @param nowMs Current tick in milliseconds.
            void begin(
                Slot& slot,
                int slotNo,
                core::Point feet,
                int kind,
                std::uint64_t nowMs
            );

            /// @brief Moves a finished light on to its next stage.
            /// @details Original: DrawEffectLight's next stage: four more sparkles from a random frame, each a little slower, then the kind's picture.
            /// @param slot Light to advance.
            /// @param nowMs Current tick in milliseconds.
            void next(Slot& slot, std::uint64_t nowMs);

            /// @brief Plays a strip of a picture at the slot's place.
            /// @param slot Light to play on.
            /// @param picture Picture name.
            /// @param frameMs Milliseconds per frame.
            /// @param first First frame (1-based).
            /// @param last Last frame (1-based).
            /// @param offsetY Pixels the strip is drawn down.
            /// @param once True for a sparkle played once, false for the kind's picture kept looping.
            /// @param nowMs Current tick in milliseconds.
            void play(
                Slot& slot,
                const char* picture,
                std::uint64_t frameMs,
                int first,
                int last,
                int offsetY,
                bool once,
                std::uint64_t nowMs
            );

            gfx::PictureLibrary& m_pictures;
            std::mt19937& m_random;
            std::map<std::int32_t, std::array<Slot, 3>> m_players;
    };

}

#endif
