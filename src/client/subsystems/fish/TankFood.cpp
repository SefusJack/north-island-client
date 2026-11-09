#include "client/subsystems/fish/TankFood.h"

namespace client {

    namespace {

        constexpr int kFrames = 4; // animation frames stacked in the food picture
        constexpr int kFrameMs = 500; // time per animation frame, ms

    }

    void TankFood::setPicture(gfx::PictureId newPicture, core::Point pictureSize) {
        picture = newPicture;
        frameWidth = pictureSize.x;
        frameHeight = pictureSize.y / kFrames;
    }

    void TankFood::draw(gfx::Canvas& canvas, core::Point origin, core::Point formSize, std::uint64_t nowMs) {
        if (posX > formSize.x || posY > formSize.y) {
            active = false;
        }
        if (!active) {
            return;
        }
        canvas.drawPicture(picture, {origin.x + posX, origin.y + posY}, core::Rect{0, frame * frameHeight, frameWidth, frameHeight});
        if (nowMs - frameMs >= kFrameMs) {
            frame = frame < kFrames - 1 ? frame + 1 : 0;
            frameMs = nowMs;
        }
    }

}
