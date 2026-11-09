#include "client/models/chat/Marquee.h"

namespace client {

    namespace {

        constexpr std::uint64_t kStepMs = 100; // ms per one-character step
        constexpr std::size_t kStartBlanks = 60; // blanks the line starts with (the 60-character line)
        constexpr std::size_t kFeedAt = 53; // next message character enters when the line is at most this long
        constexpr int kRounds = 2; // times a message runs past

    }

    void Marquee::update(std::uint64_t nowMs) {
        if (!m_running) {
            if (m_queue.empty()) {
                return;
            }
            m_running = true;
            m_round = 0;
            m_next = 0;
            m_shown = std::string(kStartBlanks, ' ');
            m_stepMs = nowMs;
        }
        if (nowMs - m_stepMs < kStepMs) {
            return;
        }
        m_stepMs = nowMs;
        if (!m_shown.empty()) {
            m_shown.erase(0, 1);
        }
        const std::string& message = m_queue.front();
        if (m_shown.size() <= kFeedAt && m_next < message.size()) {
            m_shown += message[m_next++];
        }
        if (m_shown.empty() && m_next >= message.size()) {
            if (++m_round < kRounds) {
                m_next = 0;
                m_shown = std::string(kStartBlanks, ' ');
            } else {
                m_queue.pop_front();
                m_running = false;
            }
        }
    }

}
