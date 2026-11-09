#ifndef NORTH_ISLAND_UI_TEXT_WRAP_H
#define NORTH_ISLAND_UI_TEXT_WRAP_H

#include <cstddef>
#include <string>
#include <vector>

namespace ui {

    /// @brief Splits a message into lines the way the game does.
    /// @details Original: TSe_ShortMsgPanel.WrapMsgText / GetWrapPos. A carriage return or line feed starts a new line;
    /// a longer line breaks after its last space within columns (keeping the space), or at columns when there is none,
    /// never inside a Big5 pair. With indentWrapped the lines a long line was cut into start with four spaces.
    /// @param text The message.
    /// @param columns Maximum bytes per line.
    /// @param indentWrapped Whether continuation lines start with four spaces.
    /// @return The lines.
    std::vector<std::string> wrapText(const std::string& text, std::size_t columns, bool indentWrapped = false);

    /// @brief Moves the breaks every width characters off words (a hyphen inside a word, a space before a break after a non-letter).
    /// @details Original: WrapTextLines. The result is drawn wrapped.
    /// @param text The text.
    /// @param width Characters per line.
    /// @return The text with the breaks adjusted.
    std::string wrapTextLines(const std::string& text, std::size_t width);

}

#endif
