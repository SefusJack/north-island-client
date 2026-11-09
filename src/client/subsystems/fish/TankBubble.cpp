#include "client/subsystems/fish/TankBubble.h"

namespace client {

    namespace {

        constexpr int kFrames = 4; // animation frames stacked in the bubble picture
        constexpr int kFrameMs = 500; // time per animation frame, ms
        constexpr int kTop = 10; // y the bubble rises to, px
        constexpr int kOneIn = 50; // a resting bubble starts rising with a 1 in 50 chance per update

    }

    void TankBubble::setPicture(gfx::PictureId newPicture, core::Point pictureSize) {
        picture = newPicture;
        frameWidth = pictureSize.x;
        frameHeight = pictureSize.y / kFrames;
    }

    void TankBubble::update(const Random& random) {
        if (rising) {
            if (posY > topY) {
                posY -= riseSpeed;
            } else {
                rising = false;
            }
        } else if (random(kOneIn) == 0) {
            rising = true;
            posX = startX;
            posY = startY;
            topY = kTop;
        }
    }

    void TankBubble::draw(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs) {
        if (!rising) {
            return;
        }
        canvas.drawPicture(picture, {origin.x + posX, origin.y + posY}, core::Rect{0, frame * frameHeight, frameWidth, frameHeight});
        if (nowMs - frameMs >= kFrameMs) {
            frame = frame < kFrames - 1 ? frame + 1 : 0;
            frameMs = nowMs;
        }
    }

}
