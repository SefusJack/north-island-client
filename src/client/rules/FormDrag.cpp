#include "client/rules/FormDrag.h"

#include <utility>

namespace client {

    namespace {

        constexpr int kUpright = 1; // hot key bar layout 1: upright (tall)
        constexpr int kLying = 2; // hot key bar layout 2: lying (wide)
        constexpr int kMini = 3; // hot key bar layout 3: minimised button
        constexpr int kMiniWidth = 39; // minimised hot key button width, px
        constexpr int kMiniHeight = 38; // minimised hot key button height, px
        constexpr int kBarLong = 274; // hot key bar long side, px
        constexpr int kBarShort = 56; // hot key bar short side, px
        constexpr int kEdge = 20; // distance from a screen edge that flips the bar's layout, px
        constexpr int kBottomBand = 50; // band at the screen bottom the bar must stay out of, px

    }

    core::Rect FormDrag::dragForm(core::Rect area, const core::Rect& screen) {
        if (area.y < screen.top() + 10) { // within 10 pixels of the top snaps to the top
            area.y = screen.top();
        }
        if (area.y + area.height > screen.bottom() - 60) { // bottom past 60 above the screen's bottom (540 of 600)
            area.y = screen.bottom() - 50 - area.height; // snaps to 50 above the bottom (550 of 600)
        }
        return area;
    }

    core::Rect FormDrag::fitForm(core::Rect area, bool dragging, const core::Rect& screen) {
        if (area.x < screen.left() + 10) { // within 10 pixels of the left edge
            area.x = screen.left();
        }
        if (area.x + area.width > screen.right() - 10 && area.x > screen.left()) { // within 10 pixels of the right edge
            area.x = screen.right() - area.width;
        }
        if (area.y + area.height > screen.bottom() - 35) { // bottom within 35 pixels of the screen's bottom
            area.y = screen.bottom() - 25 - area.height; // pulled to 25 above the bottom
        }
        if (area.y < screen.top() + 10 && screen.bottom() - 25 - area.height != area.y) { // within 10 pixels of the top, unless pulled to 25 above the bottom
            area.y = screen.top();
        }
        if (dragging) {
            if (area.x < screen.left() + 5) { // dragged: within 5 pixels of the left edge
                area.x = screen.left();
            }
            if (area.x + area.width > screen.right() - 5) { // dragged: within 5 pixels of the right edge
                area.x = screen.right() - area.width;
            }
            if (area.y < screen.top() + 10) { // dragged: within 10 pixels of the top
                area.y = screen.top();
            }
            if (area.y + area.height > screen.bottom() - 35) { // dragged: bottom within 35 pixels of the screen's bottom
                area.y = screen.bottom() - 25 - area.height; // pulled to 25 above the bottom
            }
        }
        return area;
    }

    core::Size FormDrag::hotKeySize(int layout) {
        if (layout == kMini) {
            return {kMiniWidth, kMiniHeight};
        }
        return layout == kLying ? core::Size{kBarLong, kBarShort} : core::Size{kBarShort, kBarLong};
    }

    FormDrag::HotKeyMove FormDrag::dragHotKeyBar(
        int layout,
        int previousLayout,
        core::Point grab,
        core::Point mouse,
        const core::Rect& screen
    ) {
        HotKeyMove move{layout, previousLayout, grab, {}};
        core::Size size = hotKeySize(layout);
        move.area = {mouse.x - grab.x, mouse.y - grab.y, size.width, size.height};
        const auto turn = [&move, &size](int to) {
            move.previousLayout = move.layout;
            move.layout = to;
            size = hotKeySize(to);
            move.area.width = size.width;
            move.area.height = size.height;
            std::swap(move.grab.x, move.grab.y);
        };
        const bool corner = mouse.x <= screen.left() + kEdge || screen.right() - kEdge <= mouse.x;
        if (layout == kLying && mouse.x <= screen.left() + kEdge) {
            turn(kUpright);
            move.area.x = screen.left();
        } else if (layout == kLying && screen.right() - kEdge <= mouse.x) {
            turn(kUpright);
            move.area.x = screen.right() - move.area.width;
        } else if (layout == kUpright && mouse.y <= screen.top() + kEdge && (!corner || previousLayout != kLying)) {
            turn(kLying);
            move.area.y = screen.top();
        } else if (
            layout == kUpright && screen.bottom() - kBottomBand - 10 <= mouse.y && (!corner || previousLayout != kLying)
        ) { // held within 60 (50 + 10) of the bottom
            turn(kLying);
            move.area.y = screen.bottom() - kBottomBand - move.area.height;
        }
        move.area = keepHotKeyBar(move.area, screen);
        return move;
    }

    core::Rect FormDrag::keepHotKeyBar(core::Rect area, const core::Rect& screen) {
        if (area.x < screen.left()) {
            area.x = screen.left();
        }
        if (area.x + area.width > screen.right()) {
            area.x = screen.right() - area.width;
        }
        if (area.y < screen.top()) {
            area.y = screen.top();
        }
        if (area.y + area.height > screen.bottom() - kBottomBand) {
            area.y = screen.bottom() - kBottomBand - area.height;
        }
        return area;
    }

}
