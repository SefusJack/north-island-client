#include "ui/Tooltip.h"

#include "gfx/TextStyle.h"

namespace ui {

    namespace {

        constexpr int kCharWidth = 8; // fixed width of one text character, px
        constexpr int kLineChars = 40; // characters per tooltip row before wrapping
        constexpr int kLineHeight = 20; // height of one tooltip row, px
        constexpr int kLowest = 30; // tooltip keeps this many px clear of the window bottom

    }

    void drawTooltipAt(gfx::Canvas& canvas, core::Point anchor, const std::string& text) {
        if (text.empty()) {
            return;
        }
        const int length = static_cast<int>(text.size());
        // rows of kLineChars characters
        const int lines = length / kLineChars + 1;
        // 6 and 4: padding of the box in pixels
        const int width = length % kLineChars * kCharWidth + 6;
        const int height = lines * kLineHeight + 4;
        core::Rect box{anchor.x - width, anchor.y - height, width, height};
        const core::Rect window = canvas.window();
        if (box.x + box.width > window.right()) {
            box.x += window.right() - (box.x + box.width) - 2;
        }
        if (box.x < window.left()) {
            box.x = window.left() + 2;
        }
        if (box.y < window.top()) {
            box.y += 50; // pushed down 50 pixels when above the window
        }
        if (box.y + box.height > window.bottom() - kLowest) {
            box.y = window.bottom() - kLowest - box.height;
        }
        // blue box fill (RGBA)
        canvas.fillRectAlpha(box, {61, 138, 249, 200});
        // blue outline (RGBA)
        canvas.drawRectOutline(box, {0, 0, 255, 255});
        // text offset 3,3 in the box; wrap width 320 (40 chars x 8) and line height 16
        canvas.drawText({box.x + 3, box.y + 3}, text, {{255, 255, 255, 255}, gfx::TextEffect::Shadow, std::nullopt}, 320, 16);
    }

    void drawHintBox(gfx::Canvas& canvas, core::Point origin, const std::string& text) {
        constexpr int kHintLineChars = 50; // characters per hint line before wrapping
        constexpr int kHintLineHeight = 23; // px height of one hint line
        if (text.empty()) {
            return;
        }
        const core::Rect window = canvas.window();
        const int length = static_cast<int>(text.size());
        const int lines = length > kHintLineChars ? (length + kHintLineChars - 1) / kHintLineChars : 1;
        // 7: horizontal padding of the hint box
        const int width = length > kHintLineChars ? kHintLineChars * kCharWidth + 7 : length * kCharWidth + 7;
        // box sits 1 pixel left of the origin and 26 pixels above it
        core::Point at{origin.x - 1, origin.y - 26};
        if (at.x + width > window.right()) {
            at.x += window.right() - (at.x + width) - 2;
        }
        if (at.x < window.left()) {
            at.x = window.left() + 2;
        }
        if (at.y < window.top()) {
            at.y += 50; // below the element instead of above it
        }
        const core::Rect box{at.x, at.y, width, lines * kHintLineHeight};
        // blue hint box fill (RGBA)
        canvas.fillRectAlpha(box, {61, 138, 249, 200});
        // dark blue outline (RGBA)
        canvas.drawRectOutline(box, {0, 0, 128, 255});
        // text offset inside the box (3, 4)
        canvas
            .drawText({at.x + 3, at.y + 4}, text, {{255, 255, 255, 255}, gfx::TextEffect::Shadow, std::nullopt}, kHintLineChars * kCharWidth, kHintLineHeight);
    }

}
