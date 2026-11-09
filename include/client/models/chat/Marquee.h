#ifndef NORTH_ISLAND_CLIENT_MODELS_CHAT_MARQUEE_H
#define NORTH_ISLAND_CLIENT_MODELS_CHAT_MARQUEE_H

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>

namespace client {

    /// @brief The system messages running along the bottom of the chat.
    /// @details Original: DrawAnnounceMarquee. Each runs past twice (FMarqueeRepeat counts the rounds from 1 and stops above 2), one character further every 100 ms, entering from the right of a 60-character line.
    class Marquee {
        public:
            /// @brief Queues a message.
            /// @param message The message to run.
            void add(const std::string& message) { m_queue.push_back(message); }

            /// @brief Advances the running message.
            /// @param nowMs Current time in ms.
            void update(std::uint64_t nowMs);

            /// @brief What shows now.
            /// @return The visible text, "" when nothing runs.
            const std::string& text() const { return m_shown; }

        private:
            std::deque<std::string> m_queue;
            std::string m_shown;
            std::size_t m_next = 0;
            int m_round = 0;
            bool m_running = false;
            std::uint64_t m_stepMs = 0;
    };

}

#endif
