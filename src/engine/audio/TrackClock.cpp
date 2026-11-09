#include "audio/TrackClock.h"

namespace audio {

    void TrackClock::start(std::uint64_t nowMs, std::uint64_t offsetMs, std::uint64_t durationMs) {
        m_startMs = nowMs;
        m_offsetMs = offsetMs;
        m_durationMs = durationMs;
        m_paused = false;
    }

    void TrackClock::pause(std::uint64_t nowMs) {
        if (!m_paused) {
            m_paused = true;
            m_pausedAtMs = nowMs;
        }
    }

    void TrackClock::resume(std::uint64_t nowMs) {
        if (m_paused) {
            m_startMs += nowMs - m_pausedAtMs;
            m_paused = false;
        }
    }

    std::uint64_t TrackClock::positionMs(std::uint64_t nowMs) const {
        if (m_durationMs == 0) {
            return 0;
        }
        const std::uint64_t now = m_paused ? m_pausedAtMs : nowMs;
        return (m_offsetMs + (now - m_startMs)) % m_durationMs;
    }

}
