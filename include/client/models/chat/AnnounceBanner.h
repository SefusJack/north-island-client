#ifndef NORTH_ISLAND_CLIENT_MODELS_CHAT_ANNOUNCE_BANNER_H
#define NORTH_ISLAND_CLIENT_MODELS_CHAT_ANNOUNCE_BANNER_H

#include <cstdint>
#include <deque>
#include <optional>
#include <string>

namespace client {

    /// @brief The prize announcements running across the top of the screen.
    /// @details Original: TTalkMsgForm's FAnnounceQueue2, DrawSystemAnnounce. The lines wait in a queue, each runs past twice at 2 pixels every 50 ms over a strip 430 pixels wide, entering from the right, with four more blanks after it. Like the game a line that comes after the first carries its four blanks twice.
    class AnnounceBanner {
        public:
            /// @brief The part of the line to draw: columns [sourceLeft, sourceRight) of its text strip, at destination (0 is the strip's left edge).
            struct Visible {
                int sourceLeft = 0;
                int sourceRight = 0;
                int destination = 0;
            };

            /// @brief Queues an announcement line.
            /// @details Original: RecvSystemAnnounce's queueing.
            /// @param line The line to run.
            void add(const std::string& line);

            /// @brief Drops the running line and the queue.
            /// @details Original: ResetTalkMessages.
            void clear();

            /// @brief Runs one frame of DrawSystemAnnounce.
            /// @param nowMs Current time in ms.
            /// @return What shows now; nothing while the queue is empty or the line is past the strip.
            std::optional<Visible> update(std::uint64_t nowMs);

            /// @brief The line running, with its blanks.
            /// @return The text.
            const std::string& text() const { return m_text; }

            static constexpr int kStripWidth = 430; // width of the banner's text strip, px

        private:
            std::deque<std::string> m_queue;
            std::string m_text;
            int m_loops = 0;
            int m_x = 0;
            int m_width = 0;
            std::optional<std::uint64_t> m_tickMs;
    };

}

#endif
