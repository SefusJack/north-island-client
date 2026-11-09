#include "client/rendering/hud/GameNumbers.h"

#include "gfx/PictureLibrary.h"

#include <array>
#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        /// @brief One size style of drawGameNumber: digit step, box size and centring width per digit.
        struct SizeStyle {
            int step;
            int width;
            int height;
            int centre;
        };

        // rows are size styles 0 to 10; columns are {digit step, box width, box height, centring width per digit}, all px
        constexpr std::array<SizeStyle, 11> kSizes{
            {{4, 4, 6, 2}, // style 0
             {8, 8, 10, 4}, // style 1
             {12, 12, 14, 6}, // style 2
             {16, 16, 17, 8}, // style 3
             {20, 20, 27, 10}, // style 4
             {26, 26, 35, 13}, // style 5
             {32, 32, 41, 16}, // style 6
             {12, 12, 15, 12}, // style 7
             {20, 20, 28, 20}, // style 8
             {10, 10, 12, 10}, // style 9
             {10, 14, 19, 7}} // style 10
        };
        constexpr int kDigitRows = 10; // digit pictures stack the digits 0-9 as 10 rows
        constexpr int kLetterRows = 26; // letter pictures stack A-Z (or a-z) as 26 rows

        /// @brief Tells whether a box lies wholly inside the window.
        /// @details Original: DelphiX's RectInRect.
        /// @param canvas Canvas providing the window rectangle.
        /// @param box The rectangle to test.
        /// @return True when the box is wholly on the screen.
        bool onScreen(const gfx::Canvas& canvas, const core::Rect& box) {
            const core::Rect window = canvas.window();
            return box.x >= window.left() && box.y >= window.top() && box.right() <= window.right() && box.bottom() <= window.bottom();
        }

    }

    void drawNativeNumber(gfx::Canvas& canvas, const std::string& font, core::Point at, int value) {
        const auto digits = canvas.pictures().find(font);
        if (!digits) {
            return;
        }
        const gfx::Picture& picture = canvas.pictures().picture(*digits);
        const int width = picture.width();
        const int height = picture.height() / kDigitRows;
        const std::string text = std::to_string(value);
        for (std::size_t i = 0; i < text.size(); ++i) {
            canvas.drawPicture(*digits, {at.x + static_cast<int>(i) * width, at.y}, core::Rect{0, (text[i] - '0') * height, width, height});
        }
    }

    void drawGameNumber(
        gfx::Canvas& canvas,
        const std::string& font,
        core::Point at,
        int value,
        int sizeStyle,
        bool centred
    ) {
        if (sizeStyle < 0 || sizeStyle >= static_cast<int>(kSizes.size())) {
            return;
        }
        const auto digits = canvas.pictures().find(font);
        if (!digits) {
            return;
        }
        const gfx::Picture& picture = canvas.pictures().picture(*digits);
        const int glyphHeight = picture.height() / kDigitRows;
        const std::string text = std::to_string(std::abs(value));
        const SizeStyle& size = kSizes[static_cast<std::size_t>(sizeStyle)];
        const int left = at.x - (centred ? static_cast<int>(text.size()) * size.centre : 0);
        for (std::size_t i = 0; i < text.size(); ++i) {
            const core::Rect box{left + static_cast<int>(i) * size.step, at.y, size.width, size.height};
            if (!onScreen(canvas, box)) {
                continue;
            }
            const int digit = text[i] - '0';
            canvas.drawPictureScaled(*digits, box, core::Rect{0, digit * glyphHeight, picture.width(), glyphHeight});
        }
    }

    void drawLevelDigits(gfx::Canvas& canvas, int level, core::Point at) {
        constexpr core::Point kDigitsOffset{30, 1}; // px: first level digit relative to the Lv word
        if (const auto word = canvas.pictures().find("Main_Word_Lv")) {
            canvas.drawPicture(*word, at);
        }
        const auto digits = canvas.pictures().find("Num_White3_1");
        if (!digits) {
            return;
        }
        const gfx::Picture& picture = canvas.pictures().picture(*digits);
        const int glyphHeight = picture.height() / kDigitRows;
        int x = at.x + kDigitsOffset.x;
        for (const char c : std::to_string(level)) {
            canvas.drawPicture(*digits, {x, at.y + kDigitsOffset.y}, core::Rect{0, (c - '0') * glyphHeight, picture.width(), glyphHeight});
            x += picture.width() - 1;
        }
    }

    void drawGameLetters(
        gfx::Canvas& canvas,
        core::Point at,
        const std::string& text,
        int font,
        float scale,
        int stepX,
        int stepY,
        bool centred
    ) {
        static const std::array<const char*, 3> kFonts{"Word_Red_64", "Word_Orange_56", "Word_Pink_55"};
        if (font < 1 || font > 6) { // valid word font numbers are 1 to 6 (pairs share a font)
            font = 1;
        }
        const std::string base = kFonts[static_cast<std::size_t>((font - 1) / 2)];
        const auto lower = canvas.pictures().find(base + "L");
        const auto upper = canvas.pictures().find(base + "U");
        const int count = static_cast<int>(text.size());
        const core::Point shift = centred ? core::Point{-(count * stepX / 2), -(count * stepY / 2)} : core::Point{};
        for (int i = count - 1; i >= 0; --i) {
            const char letter = text[static_cast<std::size_t>(i)];
            const bool isUpper = letter >= 'A' && letter <= 'Z';
            const bool isLower = letter >= 'a' && letter <= 'z';
            const auto picture = isUpper ? upper : (isLower ? lower : std::nullopt);
            if (!picture) {
                continue;
            }
            const gfx::Picture& glyphs = canvas.pictures().picture(*picture);
            const int glyphHeight = glyphs.height() / kLetterRows;
            const int row = isUpper ? letter - 'A' : letter - 'a';
            const core::Rect box{
                at.x + i * stepX + shift.x,
                at.y + i * stepY + shift.y,
                static_cast<int>(std::trunc(glyphs.width() * static_cast<double>(scale))),
                static_cast<int>(std::trunc(glyphHeight * static_cast<double>(scale)))
            };
            if (!onScreen(canvas, box)) {
                continue;
            }
            canvas.drawPictureScaled(*picture, box, core::Rect{0, row * glyphHeight, glyphs.width(), glyphHeight});
        }
    }

}
