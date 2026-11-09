#include "client/models/character/FigureAnimation.h"

namespace client {

    namespace {

        constexpr std::uint64_t kStandFrameMs = 230; // ms per frame while standing
        constexpr std::uint64_t kWalkFrameMs = 100; // ms per frame while walking

    }

    int FigureAnimation::frame(std::uint64_t nowMs, int frameCount, bool walking) {
        return frame(nowMs, frameCount, walking ? kWalkFrameMs : kStandFrameMs, false);
    }

    int FigureAnimation::frame(std::uint64_t nowMs, int frameCount, std::uint64_t frameMs, bool holdLast) {
        if (nowMs - m_frameMs > frameMs) {
            m_frameMs = nowMs;
            if (frameCount <= 0) {
                m_frame = 0;
            } else if (holdLast && m_frame + 1 >= frameCount) {
                m_frame = frameCount - 1;
            } else {
                m_frame = (m_frame + 1) % frameCount;
            }
        }
        return m_frame;
    }

}
