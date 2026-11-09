#ifndef NORTH_ISLAND_CLIENT_RENDERING_HUD_GAME_NUMBERS_H
#define NORTH_ISLAND_CLIENT_RENDERING_HUD_GAME_NUMBERS_H

#include "core/Point.h"
#include "gfx/Canvas.h"

#include <string>

namespace client {

    /// @brief Draws a number from a picture of ten digit rows, each digit stretched to a size style's box; negative numbers show without their sign.
    /// @details Original: DrawNumberByName / DrawNumber. Size styles (step/box): 0 4x6, 1 8x10, 2 12x14, 3 16x17, 4 20x27, 5 26x35, 6 32x41, 7 12x15, 8 20x28, 9 10x12, 10 14x19 one 10 apart.
    /// @param canvas Target canvas.
    /// @param font Name of the digit picture.
    /// @param at Position; the number's middle is at x when centred.
    /// @param value The number.
    /// @param sizeStyle Size style 0 to 10.
    /// @param centred True to centre the number on the point.
    void drawGameNumber(
        gfx::Canvas& canvas,
        const std::string& font,
        core::Point at,
        int value,
        int sizeStyle,
        bool centred
    );

    /// @brief Draws a number's digits at their picture's own size, one after the other from the point.
    /// @details Original: the hunting and sheep games' own digits (DrawHunterHud, DrawSheepDreamHud).
    /// @param canvas Target canvas.
    /// @param font Name of the digit picture.
    /// @param at Position of the first digit.
    /// @param value The number.
    void drawNativeNumber(gfx::Canvas& canvas, const std::string& font, core::Point at, int value);

    /// @brief Draws the word "Lv" and the level after it in the small white digits (Num_White3_1), each one pixel closer than its width.
    /// @details Original: DrawLevelDigits.
    /// @param canvas Target canvas.
    /// @param level The level number.
    /// @param at Position of the word.
    void drawLevelDigits(gfx::Canvas& canvas, int level, core::Point at);

    /// @brief Draws letters from the word fonts, scaled and one step apart.
    /// @details Original: DrawLetterText. Fonts: 1 Word_Red_64, 3 Word_Orange_56, 5 Word_Pink_55; each an L and a U picture of 26 rows.
    /// @param canvas Target canvas.
    /// @param at Position of the text (its centre when centred).
    /// @param text The text; only ASCII letters are drawn.
    /// @param font Word font number.
    /// @param scale Scale factor of each letter.
    /// @param stepX Horizontal step between letters.
    /// @param stepY Vertical step between letters.
    /// @param centred True to centre the text on the point.
    void drawGameLetters(
        gfx::Canvas& canvas,
        core::Point at,
        const std::string& text,
        int font,
        float scale,
        int stepX,
        int stepY,
        bool centred
    );

}

#endif
