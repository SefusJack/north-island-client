#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_TWEEN_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_TWEEN_H

#include "client/subsystems/battle/BattleCamera.h"
#include "client/subsystems/battle/BattleField.h"
#include "core/Point.h"

#include <array>
#include <cstdint>
#include <optional>

namespace client {

    /// @brief Draws the battle between its 30 ms ticks by blending the last two ticks' camera, fighter and layer places.
    /// @details The battle is seen one tick late. Something that moved further than kSnapPixels in a tick, or is not in both
    /// ticks, is drawn where the tick left it.
    class BattleTween {
        public:
            static constexpr int kSnapPixels = 160; // a move longer than this in one tick is not blended, px
            static constexpr int kLayerCount = 5 * (Fighter::kLayers + 1) + 2 * (Fighter::kLayers + 1);

            /// @brief Forgets the recorded ticks.
            void clear() {
                m_previous.reset();
                m_current.reset();
            }

            /// @brief Records the places after a tick.
            /// @param camera The battle camera.
            /// @param field The battle field.
            /// @param atMs Time of the tick in milliseconds.
            void record(const BattleCamera& camera, BattleField& field, std::uint64_t atMs);

            /// @brief Puts the camera, fighters and layers between the last two ticks' places for one drawing.
            /// @param camera The battle camera.
            /// @param field The battle field.
            /// @param nowMs Current time in milliseconds.
            /// @param nextBeatMs Time of the next beat in milliseconds.
            void apply(BattleCamera& camera, BattleField& field, std::uint64_t nowMs, std::uint64_t nextBeatMs);

            /// @brief Puts the tick's own places back after apply.
            /// @param camera The battle camera.
            /// @param field The battle field.
            void restore(BattleCamera& camera, BattleField& field);

            /// @brief Returns how far from the last tick's places to the newest (0 to 1) it is.
            /// @param currentMs Time of the newest tick.
            /// @param nowMs Current time in milliseconds.
            /// @param nextBeatMs Time of the next beat.
            /// @return The progress from 0 to 1.
            static double progress(std::uint64_t currentMs, std::uint64_t nowMs, std::uint64_t nextBeatMs);

            /// @brief Blends two places by `amount`, or gives `to` when they lie further apart than kSnapPixels.
            /// @param from Earlier place.
            /// @param to Later place.
            /// @param amount Blend amount from 0 to 1.
            /// @return The blended place.
            static core::Point between(core::Point from, core::Point to, double amount);

        private:
            /// @brief One fighter's places at a tick.
            struct FighterPose {
                std::int32_t id = 0;
                core::Point position;
                int jumpHeight = 0;
                int flyRise = 0;
                std::array<std::optional<core::Point>, kLayerCount> layers{};
            };

            /// @brief The camera and all fighters' places at a tick.
            struct Snapshot {
                std::uint64_t atMs = 0;
                core::Point camera;
                int farX = 0;
                std::array<std::optional<FighterPose>, BattleField::kSlots + 1> fighters{};
            };

            /// @brief Takes a snapshot of the current places.
            /// @param camera The battle camera.
            /// @param field The battle field.
            /// @param atMs Time of the snapshot in milliseconds.
            /// @return The snapshot.
            static Snapshot take(const BattleCamera& camera, BattleField& field, std::uint64_t atMs);

            /// @brief Puts the places of a snapshot onto the camera and fighters.
            /// @param camera The battle camera.
            /// @param field The battle field.
            /// @param snapshot The places to put.
            static void put(BattleCamera& camera, BattleField& field, const Snapshot& snapshot);

            std::optional<Snapshot> m_previous;
            std::optional<Snapshot> m_current;
            std::optional<Snapshot> m_saved;
    };

}

#endif
