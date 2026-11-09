#include "client/models/chat/AnnounceBanner.h"

#include <cmath>

namespace client {

    namespace {

        constexpr int kStartX = 595; // screen x where a line starts, px (right edge of the strip)
        constexpr int kStripOffset = 165; // screen x of the strip's left edge, px
        constexpr int kStepPixels = 2; // px the line moves per step
        constexpr std::uint64_t kSpeedMs = 50; // ms per step
        constexpr int kRounds = 2; // times a line runs past before the next one
        constexpr int kCharWidth = 8; // width of one text character, px
        const char* const kBlanks = "    "; // four blanks appended after each line

    }

    void AnnounceBanner::add(const std::string& line) {
        if (m_queue.empty()) {
            m_loops = 0;
            m_text = line + kBlanks;
            m_width = static_cast<int>(m_text.size()) * kCharWidth;
            m_x = kStartX;
            m_tickMs.reset();
        }
        m_queue.push_back(line + kBlanks);
    }

    void AnnounceBanner::clear() {
        m_text.clear();
        m_queue.clear();
    }

    std::optional<AnnounceBanner::Visible> AnnounceBanner::update(std::uint64_t nowMs) {
        if (m_queue.empty()) {
            return std::nullopt;
        }
        if (!m_tickMs) {
            m_tickMs = nowMs;
        }
        if (m_loops >= kRounds) {
            m_loops = 0;
            m_x = 0;
            m_queue.pop_front();
            if (m_queue.empty()) {
                return std::nullopt;
            }
            m_text = m_queue.front() + kBlanks;
            m_width = static_cast<int>(m_text.size()) * kCharWidth;
            m_x = kStartX;
            m_tickMs = nowMs;
        }
        const int at = m_x - kStripOffset;
        std::optional<Visible> shown;
        if (at > kStripWidth) {
            shown.reset();
        } else if (at + m_width > kStripWidth) {
            Visible visible{0, kStripWidth - at, at};
            if (at <= 0) {
                visible.sourceLeft = -at;
                visible.destination = 0;
            }
            shown = visible;
        } else if (at >= 0) {
            shown = Visible{0, m_width, at};
        } else if (at + m_width >= 0) {
            shown = Visible{-at, m_width, 0};
        }
        const std::uint64_t passed = nowMs > *m_tickMs ? nowMs - *m_tickMs : 0;
        const int steps = static_cast<int>(std::floor(static_cast<double>(passed) / static_cast<double>(kSpeedMs) + 0.5));
        if (steps > 0) {
            m_x -= steps * kStepPixels;
            m_tickMs = nowMs;
            if (m_x - kStripOffset + m_width < 0) {
                m_x = kStartX;
                ++m_loops;
            }
        }
        return shown;
    }

}
