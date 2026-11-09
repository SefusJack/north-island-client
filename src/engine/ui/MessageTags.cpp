#include "ui/MessageTags.h"

#include <algorithm>
#include <utility>

namespace ui {

    namespace {

        constexpr std::size_t kMostLinks = 10; // most tags kept per message
        constexpr int kMostText = 255; // longest message text position kept, characters

        /// @brief Pascal Pos: where a substring first occurs.
        /// @param sub The substring.
        /// @param text The text to search.
        /// @return The 1-based position, or 0 when not found.
        int pasPos(const std::string& sub, const std::string& text) {
            const std::size_t found = text.find(sub);
            return found == std::string::npos ? 0 : static_cast<int>(found) + 1;
        }

        /// @brief Pascal Copy: a part of a string.
        /// @param text The text.
        /// @param index The 1-based start (values below 1 count as 1).
        /// @param count The number of characters.
        /// @return The part, empty when out of range.
        std::string pasCopy(const std::string& text, int index, int count) {
            if (index < 1) {
                index = 1;
            }
            if (count <= 0 || index > static_cast<int>(text.size())) {
                return {};
            }
            return text.substr(static_cast<std::size_t>(index) - 1, static_cast<std::size_t>(count));
        }

        /// @brief Pascal Delete: removes characters from a string.
        /// @param text The text to change.
        /// @param index The 1-based start.
        /// @param count The number of characters to remove.
        void pasDelete(std::string& text, int index, int count) {
            if (index < 1 || index > static_cast<int>(text.size()) || count <= 0) {
                return;
            }
            text.erase(static_cast<std::size_t>(index) - 1, static_cast<std::size_t>(count));
        }

        /// @brief Pascal Insert: puts a string into another.
        /// @param sub The string to insert.
        /// @param text The text to change.
        /// @param index The 1-based position (clamped to the text).
        void pasInsert(const std::string& sub, std::string& text, int index) {
            index = std::clamp(index, 1, static_cast<int>(text.size()) + 1);
            text.insert(static_cast<std::size_t>(index) - 1, sub);
        }

        /// @brief Pos from an offset (the game's PosFromOffset).
        /// @param sub The substring.
        /// @param text The text to search (copied).
        /// @param start The 1-based position to search from.
        /// @return The 1-based position in the whole text, or 0 when not found.
        int posFromOffset(const std::string& sub, std::string text, int start) {
            pasDelete(text, 1, start - 1);
            int found = pasPos(sub, text);
            if (found != 0) {
                found += start - 1;
            }
            return found;
        }

        /// @brief Finds the next tag (the game's FindNextMsgTag).
        /// @param text The message text.
        /// @param kind Receives the link kind the scan saw last (1 bold, 2 red, 3 sound, 4 number, 5 name, 6 break, 7..9 expression, 11 castle owner).
        /// @param start The 1-based position to search from.
        /// @return Where the tag opens and where it closes; (0, 0) for none.
        std::pair<int, int> findNextTag(const std::string& text, int& kind, int start) {
            kind = 0;
            int tagPos = start;
            do {
                tagPos = posFromOffset("#", text, tagPos);
                if (tagPos == 0) {
                    break;
                }
                if (static_cast<int>(text.size()) >= tagPos + 1) {
                    switch (text[static_cast<std::size_t>(tagPos)]) {
                        // link kinds: 1 bold, 2 red, 3 sound, 4 number, 5 player's name, 6 line break, 7..9 expression, 11 castle owner's guild
                        case 'B':
                        case 'b': kind = 1; break; // #B: bold
                        case 'R':
                        case 'r': kind = 2; break; // #R: red
                        case 'S':
                        case 's': kind = 3; break; // #s: sound
                        case 'P':
                        case 'p': kind = 4; break; // #p: number
                        case 'N':
                        case 'n': kind = 5; break; // #n: the player's name
                        case 'E':
                        case 'e': kind = 6; break; // #e: line break
                        case 'G':
                        case 'g': kind = 11; break; // #g: the castle owner's guild
                        case 'F':
                        case 'f': // #f1..#f3: expression of the speaker
                            if (static_cast<int>(text.size()) >= tagPos + 2) {
                                switch (text[static_cast<std::size_t>(tagPos) + 1]) {
                                    case '1': kind = 7; break; // #f1: expression 1
                                    case '2': kind = 8; break; // #f2: expression 2
                                    case '3': kind = 9; break; // #f3: expression 3
                                    default: break; // other #f tags are no expression
                                }
                            }
                            break;
                        default: ++tagPos; break; // not a tag letter: go on after the '#'
                    }
                }
                if (tagPos > 1 && text[static_cast<std::size_t>(tagPos) - 2] == '/') {
                    ++tagPos;
                } else {
                    const int closePos = posFromOffset("/" + pasCopy(text, tagPos, 2), text, tagPos + 2);
                    if (closePos != 0) {
                        return {tagPos, closePos};
                    }
                    ++tagPos;
                }
            } while (tagPos != 0);
            return {0, 0};
        }

        /// @brief Counts the line breaks before both ends of a link (the game's CountCrBeforeLinkPos).
        /// @param text The message text (copied).
        /// @param from The 1-based start of the link.
        /// @param to The 1-based end of the link.
        /// @return The breaks before the start and before the end.
        std::pair<int, int> countBreaksBefore(std::string text, int from, int to) {
            int fromBreaks = 0;
            int toBreaks = 0;
            int breakPos = pasPos("\r", text);
            if (breakPos != 0) {
                do {
                    bool found = false;
                    if (from - fromBreaks > breakPos) {
                        ++fromBreaks;
                        found = true;
                    }
                    if (to - toBreaks > breakPos) {
                        ++toBreaks;
                        found = true;
                    }
                    if (!found) {
                        break;
                    }
                    pasDelete(text, breakPos, 1);
                    breakPos = pasPos("\r", text);
                } while (breakPos != 0);
            }
            return {fromBreaks, toBreaks};
        }

    }

    MessageTags::MessageTags(
        const std::string& text,
        const std::string& playerName,
        const std::string& castleOwner
    )
        : m_text(text) {
        int kind = 0;
        std::pair<int, int> span{1, 1};
        for (;;) {
            span = findNextTag(m_text, kind, span.first);
            if (span.first == 0) {
                break;
            }
            if (m_links.size() < kMostLinks) {
                Link link;
                link.kind = kind;
                link.from = span.first;
                link.to = span.second;
                if (kind == kSound) {
                    link.name = pasCopy(m_text, span.first + 2, 7); // the sound file name starts after "#s" and is 7 characters (wav1538)
                }
                m_links.push_back(link);
            }
            ++span.first;
        }
        if (m_links.empty()) {
            return;
        }
        const int count = static_cast<int>(m_links.size());
        const auto at = [this](int i) -> Link& { return m_links[static_cast<std::size_t>(i) - 1]; };
        for (int i = 1; i <= count - 1; ++i) {
            for (int j = 1; j <= count - i - 1; ++j) {
                if (at(j).from > at(j + 1).from) {
                    std::swap(at(i), at(i + 1));
                }
            }
        }
        // lengths of an opening and a closing tag: "#R" is 2, "/#R" is 3
        int openLength = 2;
        int closeLength = 3;
        for (int i = 1; i <= count; ++i) {
            if (at(i).kind == 0) {
                break;
            }
            for (int j = 1; j <= count; ++j) {
                const int jKind = at(j).kind;
                if (jKind == 0) {
                    break;
                }
                if (jKind == 3) { // kind 3: sound, "#s" + 7 file name characters
                    openLength = 9; // "#s" plus "wav1538"
                    closeLength = 3;
                } else if (jKind >= 7 && jKind <= 9) { // kinds 7..9: expression tags "#f1".."#f3"
                    openLength = 3; // "#f1"
                    closeLength = 3;
                } else {
                    openLength = 2;
                    closeLength = 3;
                }
                if (at(j).from > at(i).from) {
                    at(j).from -= openLength;
                }
                if (at(j).from > at(i).to) {
                    at(j).from -= closeLength;
                }
                if (at(j).to > at(i).from) {
                    at(j).to -= openLength;
                }
                if (at(j).to > at(i).to) {
                    at(j).to -= closeLength;
                }
            }
            pasDelete(m_text, at(i).from, openLength);
            pasDelete(m_text, at(i).to, closeLength);
        }
        for (int i = 1; i <= count; ++i) {
            const auto [fromShift, toShift] = countBreaksBefore(m_text, at(i).from, at(i).to);
            if (fromShift != 0 || toShift != 0) {
                at(i).from -= fromShift;
                at(i).to -= toShift;
            }
        }
        for (int i = 1; i <= count; ++i) {
            if (at(i).kind == 0) {
                break;
            }
            int inserted = 0;
            switch (at(i).kind) {
                case 5: // kind 5: the player's name goes in
                    pasInsert(playerName, m_text, at(i).from);
                    inserted = static_cast<int>(playerName.size());
                    break;
                case 11: // kind 11: the castle owner's guild goes in
                    if (!castleOwner.empty()) {
                        pasInsert(castleOwner, m_text, at(i).from);
                        inserted = static_cast<int>(("[ " + castleOwner + " ]").size());
                    } else {
                        m_text.clear();
                        pasInsert("No guild here", m_text, at(i).from);
                        inserted = 13; // length of "No guild here"
                    }
                    break;
                case 6: // kind 6: a line break goes in
                    pasInsert("\r", m_text, at(i).from);
                    inserted = 1;
                    break;
                default: // other kinds put nothing in
                    continue;
            }
            for (int j = 1; j <= count; ++j) {
                if (at(j).from >= at(i).from) {
                    at(j).from += inserted;
                    at(j).to += inserted;
                }
            }
        }
    }

    std::optional<std::string> MessageTags::sound() const {
        for (const Link& link : m_links) {
            if (link.kind == kSound) {
                return link.name;
            }
        }
        return std::nullopt;
    }

    int MessageTags::expression() const {
        for (const Link& link : m_links) {
            if (link.kind == 0 || link.kind == 6) { // kind 0: end of the tag list; kind 6: line break
                break;
            }
            if (link.kind >= 7 && link.kind <= 9) { // kinds 7..9: #f1..#f3
                return link.kind - 6; // kind 7 is expression 1
            }
        }
        return 0;
    }

    std::vector<MessageTags::Segment> MessageTags::segments(const std::vector<std::string>& lines, std::size_t row) const {
        std::vector<Segment> pieces;
        const std::string& line = lines[row];
        int offset = 0;
        int total = static_cast<int>(line.size());
        int curPos = 0;
        for (std::size_t j = 0; j < row; ++j) {
            const int previous = static_cast<int>(lines[j].size());
            curPos += previous;
            total = previous + total <= kMostText ? total + previous : kMostText;
            offset = previous + offset <= kMostText ? offset + previous : kMostText;
        }
        int start = curPos + 1;
        const auto draw = [&](int first, int last) {
            const int from = first - offset - 1;
            const int length = last - first + 1;
            if (from < 0 || from >= static_cast<int>(line.size()) || length <= 0) {
                return;
            }
            bool red = false;
            for (const Link& link : m_links) {
                if (link.kind == 0) {
                    break;
                }
                red = red || (link.kind == kRed && link.from <= first && link.to > first);
            }
            pieces.push_back({static_cast<std::size_t>(from), static_cast<std::size_t>(std::min<int>(length, static_cast<int>(line.size()) - from)), red});
        };
        int j = 0;
        while (static_cast<int>(line.size()) >= j) {
            ++j;
            ++curPos;
            for (const Link& link : m_links) {
                if (link.kind == 0 || link.kind == 6) {
                    break;
                }
                if (link.kind >= 7 && link.kind <= 9) {
                    continue;
                }
                if (link.from <= curPos) {
                    if (link.from == curPos && curPos - start > 0) {
                        draw(start, curPos - 1);
                        start = curPos;
                    }
                    if (link.to == curPos && curPos - start > 0) {
                        draw(start, curPos - 1);
                        start = curPos;
                        break;
                    }
                }
            }
            if (curPos >= total) {
                if (static_cast<int>(line.size()) >= start - offset) {
                    draw(start, static_cast<int>(line.size()) + offset);
                    start = curPos;
                }
                break;
            }
        }
        return pieces;
    }

}
