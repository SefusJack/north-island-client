#include "ui/TextWrap.h"

#include <algorithm>

namespace ui {

    namespace {

        /// @brief Tells whether a byte is the lead of a double byte (Big5) character.
        /// @param c The byte.
        /// @return True above 0x7F.
        bool isWide(char c) { return static_cast<unsigned char>(c) > 0x7F; } // 0x7F: last single byte (ASCII) value

        /// @brief Gives how many bytes the next line takes (original GetWrapPos).
        /// @param text Remaining text.
        /// @param columns Maximum bytes per line.
        /// @return Byte count of the next line.
        std::size_t wrapPosition(const std::string& text, std::size_t columns) {
            if (text.size() <= columns) {
                return text.size();
            }
            std::size_t next = 0;
            std::size_t lastSize = 0;
            for (std::size_t i = 0; i < columns && next < text.size(); ++i) {
                lastSize = isWide(text[next]) ? 2 : 1; // a Big5 character takes 2 bytes
                next += lastSize;
                if (next >= columns) {
                    break;
                }
            }
            if (lastSize == 2) { // the last character taken is a double byte one
                return next == columns ? columns : columns - 1;
            }
            if (!isWide(text[columns - 1]) && !isWide(text[columns])) {
                for (std::size_t i = columns; i >= 1; --i) {
                    if (text[i - 1] == ' ') {
                        return i;
                    }
                }
            }
            return columns;
        }

    }

    std::vector<std::string> wrapText(const std::string& text, std::size_t columns, bool indentWrapped) {
        std::vector<std::string> lines;
        std::string rest = text;
        while (!rest.empty()) {
            const std::size_t breakAt = rest.find_first_of("\r\n");
            if (breakAt != std::string::npos && breakAt < columns) {
                lines.push_back(rest.substr(0, breakAt));
                rest.erase(0, breakAt + 1);
                continue;
            }
            const std::size_t length = std::max<std::size_t>(1, wrapPosition(rest, columns));
            lines.push_back(rest.substr(0, length));
            rest.erase(0, length);
            if (indentWrapped && !rest.empty()) {
                rest = "    " + rest;
            }
        }
        return lines;
    }

    std::string wrapTextLines(const std::string& text, std::size_t width) {
        const auto isLetter = [](char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); };
        std::string rest = text;
        std::string result;
        while (!rest.empty()) {
            if (rest.size() <= width) {
                result += rest;
                break;
            }
            // 2: Pascal's 1-based position width - 1 is index width - 2
            const bool before = isLetter(rest[width - 2]);
            if (before && isLetter(rest[width - 1]) && isLetter(rest[width])) {
                result += rest.substr(0, width - 1) + "-";
                rest = rest.substr(width - 1);
            } else if (!before) {
                result += rest.substr(0, width - 1) + " ";
                rest = rest.substr(width - 1);
            } else {
                result += rest.substr(0, width);
                rest = rest.substr(width);
            }
        }
        return result;
    }

}
