#include "client/rendering/effects/LightEffect.h"

#include "core/Between.h"
#include "gfx/PictureLibrary.h"

#include <cmath>
#include <cstdlib>

namespace client {

    void LightEffect::start(const Setup& setup, core::Point from, std::uint64_t nowMs) {
        m_setup = setup;
        m_position = from;
        m_previous = from;
        m_active = true;
        m_atTarget = false;
        m_frameIndex = 0;
        m_frameMs = nowMs;
        m_moveMs = nowMs;
        m_arcDistance = static_cast<int>(std::lround(std::hypot(setup.target.x - from.x, setup.target.y - from.y)));
    }

    void LightEffect::update(std::uint64_t nowMs) {
        if ((m_setup.motion != Motion::ToTarget && m_setup.motion != Motion::Arc) || !m_active) {
            return;
        }
        const core::Point target = m_setup.target;
        m_previous = m_position;
        m_atTarget = m_position == target;
        const double elapsed = static_cast<double>(nowMs - m_moveMs);
        m_moveMs = nowMs;
        const int step = static_cast<int>(std::lround(elapsed * m_setup.speed));
        const double dx = std::abs(target.x - m_position.x);
        const double dy = std::abs(target.y - m_position.y);
        if (m_atTarget) {
            m_active = false;
        }
        if (step > dx && step > dy) {
            m_position = target;
            m_atTarget = true;
            return;
        }
        // 0.001: small epsilon that avoids dividing by zero
        const double slopeYX = dy > dx ? dy / (0.001 + dx) : 0;
        const double slopeXY = dy > dx ? 0 : dx / (0.001 + dy);
        const double stepX = std::min<double>(step, step / (slopeYX + 0.001));
        const double stepY = std::min<double>(step, step / (slopeXY + 0.001));
        if (m_position.x != target.x) {
            m_position.x += (m_position.x < target.x ? 1 : -1) * static_cast<int>(std::lround(stepX));
        }
        if (m_position.y != target.y) {
            m_position.y += (m_position.y < target.y ? 1 : -1) * static_cast<int>(std::lround(stepY));
        }
    }

    void LightEffect::draw(gfx::Canvas& canvas, std::uint64_t nowMs, core::Point origin) {
        constexpr int kSnapPixels = 160; // moves longer than 160 px per axis between beats are jumps, drawn without interpolation
        if (!m_active || (m_position.x <= 0 && m_position.y <= 0) || !m_setup.picture.isValid()) {
            return;
        }
        core::Point position = m_position;
        if (const auto amount = canvas.betweenBeats();
            amount && (m_setup.motion == Motion::ToTarget || m_setup.motion == Motion::Arc) && m_moveMs >= canvas.beatMs()) {
            position = core::between(m_previous, m_position, *amount, kSnapPixels);
        }
        const core::Point here{position.x - origin.x, position.y - origin.y};
        const gfx::Picture& picture = canvas.pictures().picture(m_setup.picture);
        const int width = picture.width();
        const int height = picture.height() / std::max(1, m_setup.frameCount);
        const int first = std::min(m_setup.firstFrame, m_setup.lastFrame);
        const int last = std::max(m_setup.firstFrame, m_setup.lastFrame);
        const int offset = m_setup.firstFrame <= m_setup.lastFrame ? m_frameIndex : last - 1 - m_frameIndex;
        int lift = 0;
        if (m_setup.motion == Motion::Arc) {
            if (const auto shadow = canvas.pictures().find("Shadow")) {
                canvas.drawPicture(*shadow, here, core::Rect{0, 0, 23, 11}); // size of the shadow picture: 23x11 pixels
            }
            const double remaining = std::hypot(m_setup.target.x - position.x, m_setup.target.y - position.y);
            // 360: peak height scale of the arc in pixels; 0.001 avoids dividing by zero; 0.003 flattens the lift curve
            lift = static_cast<int>(std::lround((0.5 - std::abs(std::lround(remaining) / (m_arcDistance + 0.001) - 0.5)) * 360));
            lift = static_cast<int>(std::lround(lift - lift * lift * 0.003));
        }
        const core::Point at{here.x - width / 2, here.y - lift - height + m_setup.offsetY};
        const core::Rect frame{0, (first + offset - 1) * height, width, height};
        if (m_setup.alpha == 255) { // 255: opaque, the frames are drawn as they are
            canvas.drawPicture(m_setup.picture, at, frame);
        } else {
            canvas.drawPictureLight(m_setup.picture, at, m_setup.alpha, frame);
        }
        if (nowMs - m_frameMs <= m_setup.frameMs) {
            return;
        }
        m_frameMs = nowMs;
        ++m_frameIndex;
        if (first + m_frameIndex > last) {
            if (m_setup.motion == Motion::Once) {
                m_active = false;
            } else if (m_setup.loop) {
                m_frameIndex = 0;
            } else {
                m_frameIndex = last - first;
            }
        }
    }

}
