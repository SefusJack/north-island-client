#ifndef NORTH_ISLAND_CLIENT_DATA_EMOTE_CODES_H
#define NORTH_ISLAND_CLIENT_DATA_EMOTE_CODES_H

#include <string>

namespace client {

    constexpr int kEmoteCount = 31; // number of chat emoticons (codes like "@|" and ":$"), numbered 1..31

    /// @brief Gives the two-character text code of a chat emote ("XD", ":)"), drawn as the animated icon_expre_<number>.
    /// @param number Emote number (1..31).
    /// @return The code, or an empty string for a number out of range.
    const std::string& emoteCode(int number);

    /// @brief Gives the name of a chat emote (shown as hints in the input bar).
    /// @param number Emote number (1..31).
    /// @return The name, or an empty string for a number out of range.
    const std::string& emoteName(int number);

    /// @brief Finds the emote whose code starts at a position of a text.
    /// @param text The text to look in.
    /// @param pos Index of the first character of the code.
    /// @return The emote number (1..31), or 0 when none starts there.
    int emoteAt(const std::string& text, std::size_t pos);

}

#endif
