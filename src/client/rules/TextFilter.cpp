#include "client/rules/TextFilter.h"

#include "client/data/FilterWords.h"

#include <algorithm>

namespace client {

    namespace {

        /// @brief Upper-cases the ASCII letters of a text (Original: the RTL's UpperCase).
        /// @param text The text, taken by value.
        /// @return The upper-cased text.
        std::string upper(std::string text) {
            for (char& c : text) {
                if (c >= 'a' && c <= 'z') {
                    c = static_cast<char>(c - 'a' + 'A');
                }
            }
            return text;
        }

        /// @brief Tells whether a text lacks a word (Original: TextExcludesWord).
        /// @param text The upper-cased text.
        /// @param word The upper-cased word.
        /// @return False when the word is found.
        bool excludesWord(const std::string& text, const std::string& word) {
            if (text.empty() || word.empty()) {
                return true;
            }
            const bool single = word.size() == 1;
            const auto first = static_cast<unsigned char>(word[0]);
            std::size_t at = 0;
            if (single) {
                while (at < text.size()) {
                    const auto ch = static_cast<unsigned char>(text[at]);
                    // 128: bytes above it start a two-byte Big5 character (the original's test)
                    if (ch <= 128) {
                        if (text[at] == word[0]) {
                            return false;
                        }
                        ++at;
                    } else {
                        at += 2; // skip a two-byte Big5 character
                    }
                }
                return true;
            }
            if (first <= 128) { // single-byte start (see above)
                return text.find(word) == std::string::npos;
            }
            while (at < text.size()) {
                const auto ch = static_cast<unsigned char>(text[at]);
                if (ch <= 128) { // single-byte character
                    ++at;
                } else {
                    if (text.compare(at, word.size(), word) == 0) {
                        return false;
                    }
                    at += 2; // skip a two-byte Big5 character
                }
            }
            return true;
        }

        /// @brief Tells whether a text word is one of the two Big5 spellings of "Cupid" (BadTextWords' exceptions).
        /// @param word The text word.
        /// @return True for a Cupid spelling.
        bool isCupidSpelling(const std::string& word) { return word == "\252\364\244\361\257S" || word == "\244\361\257S"; }

    }

    bool isTextClean(const std::string& text) {
        if (text.empty()) {
            return true;
        }
        const std::string shout = upper(text);
        const auto& words = textWords();
        return std::all_of(words.begin(), words.end(), [&](const char* word) { return excludesWord(shout, upper(word)); });
    }

    bool isNameClean(const std::string& text, bool staff) {
        if (text.empty()) {
            return true;
        }
        const std::string shout = upper(text);
        for (const char* part : nameParts()) {
            if (shout.find(upper(part)) != std::string::npos) {
                return false;
            }
        }
        for (const char* name : nameWords()) {
            const std::string word = upper(name);
            if (!excludesWord(shout, word) && (word != "CUPID" || !staff)) {
                return false;
            }
        }
        for (const char* textWord : textWords()) {
            const std::string word = upper(textWord);
            if (!excludesWord(shout, word) && (!isCupidSpelling(textWord) || !staff)) {
                return false;
            }
        }
        return true;
    }

}
