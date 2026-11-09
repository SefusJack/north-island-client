#ifndef NORTH_ISLAND_CORE_STRING_UTIL_H
#define NORTH_ISLAND_CORE_STRING_UTIL_H

#include <optional>
#include <string>
#include <vector>

namespace core {

    /// @brief Lower-cases a text (ASCII rules).
    /// @param text The text.
    /// @return The lower-cased text.
    std::string toLower(std::string text);

    /// @brief Compares two texts ignoring ASCII letter case.
    /// @param a First text.
    /// @param b Second text.
    /// @return True when equal ignoring case.
    bool equalsIgnoreCase(const std::string& a, const std::string& b);

    /// @brief Removes white space from both ends.
    /// @param text The text.
    /// @return The trimmed text.
    std::string trim(const std::string& text);

    /// @brief Splits a text at every separator, keeping empty parts.
    /// @param text The text.
    /// @param separator Separator character.
    /// @return The parts (at least one).
    std::vector<std::string> split(const std::string& text, char separator);

    /// @brief Parses the whole text as a decimal integer (optional sign), nothing else.
    /// @param text The text.
    /// @return The value, nothing when malformed or out of the 32-bit range.
    std::optional<int> parseInt(const std::string& text);

    /// @brief Splits a text file's contents into lines (CR LF, LF or CR), as Delphi's TStrings does.
    /// @param text The contents.
    /// @return The lines.
    std::vector<std::string> splitLines(const std::string& text);

}

#endif
