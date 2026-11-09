#include "client/rendering/hud/InfoDigits.h"

#include "gfx/PictureLibrary.h"

#include <string>

namespace client {

    void drawInfoDigits(gfx::Canvas& canvas, core::Point at, int value) {
        constexpr int kRows = 11; // Num_MainInfo stacks 11 rows: digits 0-9 and the slash
        const auto digits = canvas.pictures().find("Num_MainInfo");
        if (!digits) {
            return;
        }
        const gfx::Picture& picture = canvas.pictures().picture(*digits);
        const int height = picture.height() / kRows;
        int x = at.x;
        for (const char digit : std::to_string(value)) {
            canvas.drawPicture(*digits, {x, at.y}, core::Rect{0, (digit - '0') * height, picture.width(), height});
            x += picture.width();
        }
    }

}
