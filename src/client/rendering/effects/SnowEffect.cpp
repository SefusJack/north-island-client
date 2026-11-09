#include "client/rendering/effects/SnowEffect.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        constexpr std::uint64_t kSpawnMs = 500; // one new snowflake every 500 ms
        constexpr int kScreenWidth = 800; // game view width, px
        constexpr int kScreenHeight = 600; // game view height, px
        constexpr int kFrameCount = 4; // frames stacked vertically in each icon_Snow picture

        /// @brief Rounds like Delphi's Round: halves go to the even number.
        /// @param value Value to round.
        /// @return The rounded integer.
        int roundHalfEven(double value) { return static_cast<int>(std::nearbyint(value)); }

    }

    SnowEffect::SnowEffect(
        gfx::PictureLibrary& pictures
    )
        : m_library(pictures) {
        for (std::size_t i = 0; i < m_pictures.size(); ++i) {
            m_pictures[i] = pictures.find("icon_Snow" + std::to_string(i + 1)).value_or(gfx::PictureId{});
        }
    }

    int SnowEffect::random(int count) { return std::uniform_int_distribution<int>(0, count - 1)(m_random); }

    void SnowEffect::clear() {
        m_started = false;
        for (SnowFlake& flake : m_flakes) {
            flake.active = false;
        }
    }

    void SnowEffect::reset(SnowFlake& flake, core::Point view, std::uint64_t nowMs) {
        flake.frame = 0;
        flake.frameDelay = random(3) * 100 + 200; // frame delay 200, 300 or 400 ms
        flake.speedLevel = random(3) + 1; // speed level 1..3
        flake.position = {random(kScreenWidth) + view.x, view.y};
        flake.target.y = view.y + kScreenHeight;
        const int roll = random(100);
        flake.picture = roll >= 60 ? 1 : 0; // 40 percent chance of the second picture
        // drifts up to 4 steps of 50 pixels to either side
        flake.target.x = random(2) == 0 ? flake.position.x + random(5) * 50 : flake.position.x - random(5) * 50;
        flake.frameMs = nowMs;
        flake.shakeRange = random(20); // shake range 0..19 pixels
        flake.arrived = false;
        flake.shakeRight = true;
        flake.shakePhase = 0;
        flake.shakeOffset = 0;
    }

    void SnowEffect::move(SnowFlake& flake, std::uint64_t nowMs) {
        double speed = 0.015; // default speed in pixels per ms
        switch (flake.speedLevel) {
            case 1: speed = 0.01; break; // speed level 1: 0.01 pixels per ms
            case 2: speed = 0.015; break; // speed level 2: 0.015 pixels per ms
            case 3: speed = 0.02; break; // speed level 3: 0.02 pixels per ms
            default: break; // other levels keep the default speed
        }
        const int step = roundHalfEven(static_cast<double>(nowMs - flake.frameMs) * speed);
        const double distX = std::abs(flake.target.x - flake.position.x);
        const double distY = std::abs(flake.target.y - flake.position.y);
        if (step > distX && step > distY) {
            flake.position = flake.target;
            flake.arrived = true;
            return;
        }
        double ratioYX = 0;
        double ratioXY = 0;
        if (distY > distX) {
            ratioYX = distY / (distX + 0.001); // 0.001: epsilon that avoids dividing by zero
        } else {
            ratioXY = distX / (distY + 0.001); // 0.001: epsilon that avoids dividing by zero
        }
        const double stepX = std::min<double>(step, step / (ratioYX + 0.001)); // 0.001: epsilon that avoids dividing by zero
        const double stepY = std::min<double>(step, step / (ratioXY + 0.001)); // 0.001: epsilon that avoids dividing by zero
        int dx = 0;
        int dy = 0;
        if (flake.position.x < flake.target.x) {
            dx = roundHalfEven(stepX);
        } else if (flake.position.x > flake.target.x) {
            dx = -roundHalfEven(stepX);
        }
        if (flake.position.y < flake.target.y) {
            dy = roundHalfEven(stepY);
        } else if (flake.position.y > flake.target.y) {
            dy = -roundHalfEven(stepY);
        }
        flake.position.x += dx;
        flake.position.y += dy;
        if (nowMs - flake.frameMs >= static_cast<std::uint64_t>(flake.frameDelay)) {
            flake.frame = flake.frame + 1 > kFrameCount - 1 ? 0 : flake.frame + 1;
            flake.frameMs = nowMs;
        }
    }

    void SnowEffect::update(core::Point view, std::uint64_t nowMs) {
        if (!m_started) {
            m_started = true;
            m_spawnMs = nowMs;
        }
        if (nowMs - m_spawnMs >= kSpawnMs) {
            const auto free = std::find_if(m_flakes.begin(), m_flakes.end(), [](const SnowFlake& flake) { return !flake.active; });
            if (free != m_flakes.end()) {
                reset(*free, view, nowMs);
                free->active = true;
                m_spawnMs = nowMs;
            }
        }
        for (SnowFlake& flake : m_flakes) {
            if (!flake.active) {
                continue;
            }
            move(flake, nowMs);
            if (flake.arrived) {
                flake.active = false;
            }
        }
    }

    void SnowEffect::shake(SnowFlake& flake) {
        const int quarter = flake.shakeRange / 4; // 4: a swing is made in quarter steps of the range
        if (flake.shakeRight) {
            switch (flake.shakePhase) {
                case 0: // phase 0: swinging out to the right
                    flake.shakeOffset += quarter;
                    if (flake.shakeOffset > flake.shakeRange) {
                        flake.shakePhase = 1;
                        flake.shakeOffset = flake.shakeRange;
                    }
                    break;
                case 1: flake.shakePhase = 2; break; // phase 1: hold one frame at the right end
                case 2: // phase 2: swinging back to the middle
                    flake.shakeOffset -= quarter;
                    if (flake.shakeOffset < 0) {
                        flake.shakeOffset = 0;
                        flake.shakeRight = false;
                        flake.shakePhase = 0;
                    }
                    break;
                default: break; // unknown phase: nothing
            }
            return;
        }
        switch (flake.shakePhase) {
            case 0: // phase 0: swinging out to the left
                flake.shakeOffset -= quarter;
                if (-flake.shakeRange > flake.shakeOffset) {
                    flake.shakePhase = 1;
                    flake.shakeOffset = -flake.shakeRange;
                }
                break;
            case 1: flake.shakePhase = 2; break; // phase 1: hold one frame at the left end
            case 2: // phase 2: swinging back to the middle
                flake.shakeOffset += quarter;
                if (flake.shakeOffset >= 0) {
                    flake.shakeOffset = 0;
                    flake.shakeRight = true;
                    flake.shakePhase = 0;
                }
                break;
            default: break; // unknown phase: nothing
        }
    }

    void SnowEffect::draw(gfx::Canvas& canvas, core::Point view) {
        if (!m_pictures[0].isValid()) {
            return;
        }
        const gfx::Picture& first = m_library.picture(m_pictures[0]);
        const int width = first.width();
        const int height = first.height() / kFrameCount;
        for (SnowFlake& flake : m_flakes) {
            if (!flake.active || flake.arrived) {
                continue;
            }
            shake(flake);
            const gfx::PictureId picture = m_pictures[static_cast<std::size_t>(flake.picture)];
            if (!picture.isValid()) {
                continue;
            }
            const core::Rect frame{0, flake.frame * height, width, height};
            canvas.drawPicture(picture, {flake.position.x + flake.shakeOffset - view.x, flake.position.y - view.y}, frame);
        }
    }

}
