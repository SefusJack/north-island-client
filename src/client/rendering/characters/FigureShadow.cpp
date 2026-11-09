#include "client/rendering/characters/FigureShadow.h"

#include "core/Rect.h"
#include "gfx/PictureLibrary.h"

namespace client {

    void drawFigureShadow(gfx::Canvas& canvas, core::Point feet, int kind) {
        if (kind == 1) { // shadow kind 1: the large monster shadow
            if (const auto shadow = canvas.pictures().find("Monster_Shadow")) {
                canvas.drawPicture(*shadow, {feet.x - 47, feet.y - 18}); // monster shadow offset from the feet: 47 left, 18 up
            }
        } else if (kind == 0) { // shadow kind 0: the small dithered shadow of people
            if (const auto shadow = canvas.pictures().find("Shadow")) {
                // people shadow offset from the feet: 15 left, 4 up; source rect (49,88)-(80,105) of the "Shadow" picture
                canvas.drawPicture(*shadow, {feet.x - 15, feet.y - 4}, core::Rect::fromEdges(49, 88, 80, 105));
            }
        }
    }

}
