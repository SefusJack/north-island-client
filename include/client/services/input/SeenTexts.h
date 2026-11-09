#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_SEEN_TEXTS_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_SEEN_TEXTS_H

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>

namespace client {

    /// @brief The last chat and message lines a scene showed, for a test script that waits for one (waitmsg).
    class SeenTexts {
        public:
            static constexpr std::size_t kKept = 200; // lines kept

            /// @brief Remembers a shown line, dropping the oldest beyond kKept.
            /// @param text The line.
            void add(const std::string& text);

            /// @brief How many lines were added so far.
            /// @return The count.
            std::uint64_t count() const { return m_count; }

            /// @brief Tells whether a line added after the first `mark` lines (of those still kept) contains the text.
            /// @param mark Number of lines to skip, as returned by count().
            /// @param text Text to search for.
            /// @return True when such a line exists.
            bool containsSince(std::uint64_t mark, const std::string& text) const;

        private:
            std::deque<std::string> m_texts;
            std::uint64_t m_count = 0;
    };

}

#endif
