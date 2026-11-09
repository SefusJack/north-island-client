#ifndef NORTH_ISLAND_AUDIO_TRACK_CLOCK_H
#define NORTH_ISLAND_AUDIO_TRACK_CLOCK_H

#include <cstdint>

namespace audio {

    /// @brief Keeps time of where a looped music track is, since SDL_mixer does not tell how far it has got.
    /// @details Original: GameSound.FStreams[0].Position, which a movie saves at its start as FMusicResumePos and sets again when it ends. The time stands while paused.
    class TrackClock {
        public:
            /// @brief Begins a track of the given length now.
            /// @param nowMs Current time in ms.
            /// @param offsetMs Where in the track it begins.
            /// @param durationMs Track length in ms.
            void start(std::uint64_t nowMs, std::uint64_t offsetMs, std::uint64_t durationMs);

            /// @brief Stops the clock.
            /// @param nowMs Current time in ms.
            void pause(std::uint64_t nowMs);

            /// @brief Lets the clock run again after a pause.
            /// @param nowMs Current time in ms.
            void resume(std::uint64_t nowMs);

            /// @brief Gives the place in the track; the track goes round when it ends.
            /// @param nowMs Current time in ms.
            /// @return Position in ms from the track's start.
            std::uint64_t positionMs(std::uint64_t nowMs) const;

        private:
            std::uint64_t m_startMs = 0;
            std::uint64_t m_offsetMs = 0;
            std::uint64_t m_durationMs = 0;
            std::uint64_t m_pausedAtMs = 0;
            bool m_paused = false;
    };

}

#endif
