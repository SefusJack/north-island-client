#include "client/subsystems/fish/TankFish.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        constexpr int kFormMaxX = 550; // tank width a fish may swim in, px
        constexpr int kFormMaxY = 310; // tank height a fish may swim in, px
        constexpr int kLowest = 250; // lowest destination y, px
        constexpr int kLeast = 10; // smallest destination x and y, px
        constexpr int kFrameMs = 500; // time per animation frame, ms
        constexpr int kSurfaceMs = 1000; // longest a surfacing fish keeps rising, ms
        constexpr int kLongestSwim = 100; // px swum sideways before the "in tank" fish surfaces; also a destination y limit
        constexpr int kFrames = 4; // animation frames across the fish picture (2 rows: facing left / right)

    }

    void TankFish::setPicture(gfx::PictureId newPicture, core::Point pictureSize, const Random& random) {
        active = newPicture.isValid();
        picture = newPicture;
        frameWidth = pictureSize.x / kFrames;
        frameHeight = pictureSize.y / 2;
        maxX = kFormMaxX - frameWidth;
        maxY = kFormMaxY - frameHeight;
        posX = random(maxX);
        posY = random(maxY);
        destX = random(maxX);
        destY = random(maxY);
        speedBonusX = 0;
        speedBonusY = 0;
    }

    void TankFish::clear() {
        picture = {};
        frameWidth = 0;
        frameHeight = 0;
        maxX = 0;
        maxY = 0;
        posX = 0;
        posY = 0;
        destX = 0;
        destY = 0;
        speedBonusX = 0;
        speedBonusY = 0;
        active = false;
    }

    void TankFish::updateSurfacing(std::uint64_t nowMs) {
        ++posY;
        if (posY >= maxY) {
            surfacing = false;
        }
        if (nowMs - motionMs >= kSurfaceMs) {
            surfacing = false;
        }
    }

    void TankFish::calcStep() {
        double across = std::abs(posX - destX);
        double down = std::abs(posY - destY);
        if (across == 0 && down == 0) {
            return;
        }
        if (across > down) {
            const double ratio = down / across;
            across = 5.0; // five pixels along the longer way
            down = across * ratio;
        } else {
            const double ratio = across / down;
            down = 5.0; // five pixels along the longer way
            across = across * ratio;
        }
        // at most 3 pixels across and 2 down a frame (before the food bonus)
        stepX = std::min(static_cast<int>(std::nearbyint(across)), 3);
        stepY = std::min(static_cast<int>(std::nearbyint(down)), 2);
        if (stepX > 0) {
            stepX += speedBonusX;
        }
        if (stepY > 0) {
            stepY += speedBonusY;
        }
    }

    void TankFish::update(const Random& random, std::uint64_t nowMs) {
        if (inTank) {
            if (surfacing) {
                updateSurfacing(nowMs);
                return;
            }
            if (swimDist >= kLongestSwim) {
                surfacing = true;
                motionMs = nowMs;
                swimDist = 0;
                return;
            }
        }
        if (posX == destX) {
            destX = random(maxX);
            destY = random(maxY);
            swimDist = 0;
            if (minY != 0 && destY <= kLongestSwim) {
                destY = minY;
            }
            speedBonusX = 0;
            speedBonusY = 0;
            destY = std::max(destY, kLeast);
            destX = std::max(destX, kLeast);
            destY = std::min(destY, kLowest);
        }
        facing = posX > destX ? 0 : 1;
        calcStep();
        const bool left = posX > destX;
        const bool right = posX < destX;
        const bool up = posY > destY;
        const bool downward = posY < destY;
        if (!left && !right) {
            if (up) {
                posY = std::max(posY - stepY, destY);
            } else if (downward) {
                posY = std::min(posY + stepY, destY);
            }
        } else if (posY == destY) {
            posX = left ? std::max(posX - stepX, destX) : std::min(posX + stepX, destX);
            swimDist += stepX;
        } else {
            posX = left ? std::max(posX - stepX, destX) : std::min(posX + stepX, destX);
            swimDist += stepX;
            posY = up ? std::max(posY - stepY, destY) : std::min(posY + stepY, destY);
        }
    }

    void TankFish::draw(gfx::Canvas& canvas, core::Point origin, int formWidth, std::uint64_t nowMs) {
        if (!active) {
            return;
        }
        int clipX = 0;
        if (posX + frameWidth > formWidth) {
            clipX = posX + frameWidth - formWidth;
            facing = 1;
        }
        const core::Rect source{frame * frameWidth, facing * frameHeight, frameWidth, frameHeight};
        canvas.drawPicture(picture, {origin.x + posX - clipX, origin.y + posY}, source);
        if (nowMs - frameMs >= kFrameMs) {
            frame = frame < kFrames - 1 ? frame + 1 : 0;
            frameMs = nowMs;
        }
    }

}
