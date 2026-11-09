#ifndef NORTH_ISLAND_CLIENT_SERVICES_FRAME_BEAT_CLOCK_H
#define NORTH_ISLAND_CLIENT_SERVICES_FRAME_BEAT_CLOCK_H

#include "client/services/frame/FrameBeat.h"

#include <cstdint>

namespace client {

    /// @brief Beat grid of the free frame mode (--fps): the game update runs on a 30 ms grid while frames are drawn in between.
    /// @details Original: a missed beat is not made up, as with the original's timer which waits for the last tick to be done.
    class BeatClock {
        public:
            /// @brief Tells whether an update is due and, when it is, sets the next beat.
            /// @param nowMs Current time in milliseconds.
            /// @return True when an update is due (always on the first call).
            bool due(std::uint64_t nowMs) {
                if (m_started && nowMs < m_nextMs) {
                    return false;
                }
                if (!m_started) {
                    m_started = true;
                    m_nextMs = nowMs;
                }
                while (m_nextMs <= nowMs) {
                    m_nextMs += FrameBeat::kBeatMs;
                }
                return true;
            }

            /// @brief Moves the next beat to the first grid beat after a tick that held the clock (the battle's sleeps).
            /// @param heldMs Time in milliseconds until which the tick held the clock.
            void deferPast(std::uint64_t heldMs) {
                while (m_started && m_nextMs <= heldMs) {
                    m_nextMs += FrameBeat::kBeatMs;
                }
            }

            /// @brief Time of the next beat.
            /// @return Next beat in milliseconds.
            std::uint64_t nextMs() const { return m_nextMs; }

        private:
            bool m_started = false;
            std::uint64_t m_nextMs = 0;
    };

}

#endif
