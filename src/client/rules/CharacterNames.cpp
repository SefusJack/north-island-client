#include "client/rules/CharacterNames.h"

#include "client/rules/TextFilter.h"

#include <algorithm>
#include <cctype>

namespace client {

    namespace {

        /// @brief Tells whether a character is an ASCII letter or digit.
        /// @param c The character.
        /// @return True for ASCII letters and digits.
        bool isLetterOrDigit(char c) {
            const auto u = static_cast<unsigned char>(c);
            return u < 128 && std::isalnum(u) != 0; // ASCII range
        }

    }

    std::optional<std::string> characterNameProblem(const std::string& name) {
        if (name.empty()) {
            return "No valid name.";
        }
        if (name.size() > kMaxCharacterNameLength) {
            return "Name is too long";
        }
        if (!std::all_of(name.begin(), name.end(), isLetterOrDigit)) {
            return "Don't use special characters";
        }
        if (!isNameClean(name, false)) {
            return kIllegalWords;
        }
        return std::nullopt;
    }

}
