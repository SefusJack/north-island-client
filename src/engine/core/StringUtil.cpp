#include "core/StringUtil.h"

#include <algorithm>
#include <cctype>

namespace core {

    std::string toLower(std::string text) {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return text;
    }

    bool equalsIgnoreCase(const std::string& a, const std::string& b) { return a.size() == b.size() && toLower(a) == toLower(b); }

    std::string trim(const std::string& text) {
        const auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
        auto first = std::find_if_not(text.begin(), text.end(), isSpace);
        auto last = std::find_if_not(text.rbegin(), text.rend(), isSpace).base();
        return first < last ? std::string(first, last) : std::string();
    }

    std::vector<std::string> split(const std::string& text, char separator) {
        std::vector<std::string> parts;
        std::string current;
        for (char c : text) {
            if (c == separator) {
                parts.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
        parts.push_back(current);
        return parts;
    }

    std::optional<int> parseInt(const std::string& text) {
        if (text.empty()) {
            return std::nullopt;
        }
        std::size_t i = 0;
        bool negative = false;
        if (text[0] == '-' || text[0] == '+') {
            negative = text[0] == '-';
            i = 1;
        }
        if (i == text.size()) {
            return std::nullopt;
        }
        long long value = 0;
        for (; i < text.size(); ++i) {
            if (text[i] < '0' || text[i] > '9') {
                return std::nullopt;
            }
            value = value * 10 + (text[i] - '0'); // decimal digit shift
            if (value > 2147483648LL) { // 2^31, largest magnitude still allowed (for INT_MIN)
                return std::nullopt;
            }
        }
        value = negative ? -value : value;
        if (value > 2147483647LL) { // INT_MAX
            return std::nullopt;
        }
        return static_cast<int>(value);
    }

    std::vector<std::string> splitLines(const std::string& text) {
        std::vector<std::string> lines;
        std::string current;
        for (std::size_t i = 0; i < text.size(); ++i) {
            const char c = text[i];
            if (c == '\r' || c == '\n') {
                lines.push_back(current);
                current.clear();
                if (c == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
                    ++i;
                }
            } else {
                current += c;
            }
        }
        if (!current.empty()) {
            lines.push_back(current);
        }
        return lines;
    }

}
