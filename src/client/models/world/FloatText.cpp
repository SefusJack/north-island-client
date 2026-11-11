#include "client/models/world/FloatText.h"

namespace client {

    namespace {

        constexpr std::uint64_t kRiseMs = 100; // time between two rise steps, ms
        constexpr int kRiseStep = 10; // px the text rises per step
        constexpr int kLastRise = 100; // total rise in px after which the text disappears

    }

    void FloatText::show(const std::string& text, std::uint64_t nowMs) {
        m_text = text;
        m_lastMs = nowMs;
        m_rise = 0;
    }

    std::optional<FloatText::Shown> FloatText::update(std::uint64_t nowMs) {
        if (m_text.empty()) {
            return std::nullopt;
        }
        if (nowMs - m_lastMs >= kRiseMs) {
            m_rise += kRiseStep;
            m_lastMs = nowMs;
        }
        Shown shown{m_text, m_rise};
        if (m_rise > kLastRise) {
            m_text.clear();
        }
        return shown;
    }

}
