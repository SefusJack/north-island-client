#include "client/rendering/characters/PortraitBlink.h"

namespace client {

    namespace {

        constexpr std::uint64_t kClosedMs = 50; // eyes stay closed 50 ms
        constexpr std::uint64_t kOpenMinMs = 6000; // eyes stay open at least 6 s
        constexpr std::uint64_t kOpenRandomMs = 2000; // plus a random 0..1999 ms

    }

    std::uint64_t PortraitBlink::openTimeMs() { return kOpenMinMs + std::uniform_int_distribution<std::uint64_t>(0, kOpenRandomMs - 1)(m_random); }

    void PortraitBlink::restart() {
        m_closed = false;
        m_changeMs = 0;
    }

    int PortraitBlink::frame(std::uint64_t nowMs) {
        if (m_changeMs == 0) {
            m_changeMs = nowMs + openTimeMs();
        } else if (nowMs >= m_changeMs) {
            m_closed = !m_closed;
            m_changeMs = nowMs + (m_closed ? kClosedMs : openTimeMs());
        }
        return m_closed ? 1 : 0; // portrait frame 1 = eyes closed, 0 = open
    }

}
