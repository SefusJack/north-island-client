#ifndef NORTH_ISLAND_CLIENT_RULES_TEXT_FILTER_H
#define NORTH_ISLAND_CLIENT_RULES_TEXT_FILTER_H

#include <string>

namespace client {

    /// @brief Tells whether a text holds none of the refused words (Original: DZ_Game.pas IsTextClean).
    /// @details Case does not matter. The game's Big5 characters take two bytes and a word is only found where one begins.
    /// @param text The text to check.
    /// @return True when the text is clean.
    bool isTextClean(const std::string& text);

    /// @brief Tells whether a name is clean (Original: DZ_Game.pas IsNameClean).
    /// @details Also checks the parts of BadNameParts anywhere, and the longer list of names. Staff ids (GetIdCategory 105..110) are exempt from "Cupid" in the name list and from two Big5 words of the text list; this build's lists hold neither, so everyone is refused the same.
    /// @param text The name to check.
    /// @param staff True for a staff id.
    /// @return True when the name is clean.
    bool isNameClean(const std::string& text, bool staff);

    constexpr const char* kIllegalWords = "Don't use illegal words";

}

#endif
