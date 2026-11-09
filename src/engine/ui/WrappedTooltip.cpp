#include "ui/WrappedTooltip.h"

#include "gfx/TextStyle.h"
#include "ui/HintArea.h"

namespace ui {

    namespace {

        constexpr int kCharWidth = 8; // fixed width of one text character, px
        constexpr int kBottomMargin = 30; // tooltip keeps this many px clear of the window bottom

    }

    void drawWrappedTooltip(
        gfx::Canvas& canvas,
        core::Point at,
        const std::string& text,
        int lineChars,
        int lineHeight,
        core::Color textColor,
        core::Color background,
        core::Color border
    ) {
        if (text.empty()) {
            return;
        }
        const std::string wrapped = HintArea::wrap(text, lineChars);
        const int lines = static_cast<int>(wrapped.size()) / lineChars + 1;
        // 6 and 4: padding of the box in pixels
        core::Rect box{at.x, at.y, lineChars * kCharWidth + 6, lines * lineHeight + 4};
        const core::Rect window = canvas.window();
        if (box.right() > window.right()) {
            box.x += window.right() - box.right() - 2;
        }
        if (box.x < window.left()) {
            box.x = window.left() + 2;
        }
        if (box.y < window.top()) {
            box.y = window.top() + 2;
        }
        if (box.bottom() > window.bottom() - kBottomMargin) {
            box.y += window.bottom() - kBottomMargin - box.bottom();
        }
        canvas.fillRectAlpha(box, background);
        canvas.drawRectOutline(box, border);
        // text offset 3,3 inside the box
        canvas.drawText({box.x + 3, box.y + 3}, wrapped, {textColor, gfx::TextEffect::Shadow, std::nullopt}, lineChars * kCharWidth, lineHeight);
    }

}
