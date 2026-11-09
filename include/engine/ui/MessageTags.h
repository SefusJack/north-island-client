#ifndef NORTH_ISLAND_UI_MESSAGE_TAGS_H
#define NORTH_ISLAND_UI_MESSAGE_TAGS_H

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace ui {

    /// @brief Parses the tags in the game's message texts (#R red, #B bold, #n name, #g guild, #e break, #s sound, #f1..#f3 expression).
    /// @details Original: TSe_ShortMsgPanel.ParseMsgLinks; positions are 1-based and keep the game's quirks.
    class MessageTags {
        public:
            static constexpr int kBold = 1;
            static constexpr int kRed = 2;
            static constexpr int kSound = 3;

            /// @brief One tag span: its kind and where it starts and ends in the text without tags.
            struct Link {
                int kind = 0;
                int from = 0;
                int to = 0;
                std::string name;
            };

            /// @brief A piece of a line drawn alike (its bytes and whether it is red).
            struct Segment {
                std::size_t from = 0;
                std::size_t length = 0;
                bool red = false;
            };

            /// @brief Parses the tags of a text, cuts them out and puts the names in.
            /// @param text The message text with tags.
            /// @param playerName The player's name for "#n".
            /// @param castleOwner The castle owner's guild for "#g" (empty: "No guild here").
            MessageTags(
                const std::string& text,
                const std::string& playerName,
                const std::string& castleOwner
            );

            /// @brief The text with the tags taken out and the names put in.
            /// @return The plain text.
            const std::string& text() const { return m_text; }

            /// @brief Whether the text had any tag.
            /// @return True when it had.
            bool tagged() const { return !m_links.empty() && m_links.front().kind != 0; }

            /// @brief The file (without ".wav") of the sound tag.
            /// @return The file name, or nothing without a sound tag.
            std::optional<std::string> sound() const;

            /// @brief The speaker's expression the first #f1..#f3 tag asks for; a line break tag ends the search.
            /// @details Original: FSpeakerExprKind.
            /// @return 1..3, or 0 for none.
            int expression() const;

            /// @brief How one line of the wrapped text is cut into pieces of one colour.
            /// @details Original: DrawControl / DrawMsgSegment of the message panel.
            /// @param lines The wrapped lines of the text.
            /// @param row The line to cut.
            /// @return The pieces in order.
            std::vector<Segment> segments(const std::vector<std::string>& lines, std::size_t row) const;

        private:
            std::vector<Link> m_links;
            std::string m_text;
    };

}

#endif
