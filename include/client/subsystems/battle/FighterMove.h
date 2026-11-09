#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_FIGHTER_MOVE_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_FIGHTER_MOVE_H

#include "core/Point.h"

#include <cstdint>

namespace client {

    /// @brief A fighter's own mover: from where it stands towards its destination, which is its cell.
    /// @details Original: TFightHum.UpdateMove, run for every fighter each tick before the actions; moves by
    /// round(milliseconds since its last step * FMoveSpeed) pixels. Every skill motion sets the speed (SetMoveSpeed: 0.6 on
    /// the ground, 0.8 jumping, or the point's slope) and writes the fighter's place itself each tick, so it shows only where
    /// no motion does: the attacker back at its cell while the camera is still on its way home.
    class FighterMove {
        public:
            static constexpr double kDefaultSpeed = 0.36;

            /// @brief Sets the move speed.
            /// @details Original: SetMoveSpeed.
            /// @param speed Speed in pixels per millisecond.
            void setSpeed(double speed) { m_speed = speed; }

            /// @brief Returns the move speed.
            /// @return Speed in pixels per millisecond.
            double speed() const { return m_speed; }

            /// @brief Restarts the move timer.
            /// @details Original: FMoveTick := GetTickCount.
            /// @param nowMs Current time in milliseconds.
            void restart(std::uint64_t nowMs) { m_lastMs = nowMs; }

            /// @brief Makes one UpdateMove step from `screen` towards `dest`.
            /// @param screen Current screen position.
            /// @param dest Destination.
            /// @param nowMs Current time in milliseconds.
            /// @return The new position.
            core::Point step(core::Point screen, core::Point dest, std::uint64_t nowMs);

        private:
            double m_speed = kDefaultSpeed;
            std::uint64_t m_lastMs = 0;
    };

}

#endif
