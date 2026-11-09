#include "client/models/world/WalkMark.h"

namespace client {

    namespace {

        constexpr std::uint64_t kFrameMs = 120; // time per animation frame, ms
        constexpr int kLastFrame = 4; // last frame of the destination mark animation (frames 1..4)

    }

    void WalkMark::start(core::Point destination) {
        m_active = true;
        m_position = destination;
        m_frame = 1;
    }

    int WalkMark::update(std::uint64_t nowMs) {
        if (!m_active) {
            return 0;
        }
        if (!m_tickMs || nowMs - *m_tickMs >= kFrameMs) {
            if (m_frame == kLastFrame) {
                m_frame = 1;
                m_active = false;
                return 0;
            }
            m_shown = ++m_frame;
            m_tickMs = nowMs;
        }
        return m_shown;
    }

}
