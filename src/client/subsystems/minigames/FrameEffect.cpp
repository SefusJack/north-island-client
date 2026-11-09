#include "client/subsystems/minigames/FrameEffect.h"

#include <cstdlib>

namespace client {

    void FrameEffect::start(
        gfx::PictureId picture,
        core::Point topLeft,
        core::Size size,
        int segmentLength,
        int thickness
    ) {
        m_picture = picture;
        m_topLeft = topLeft;
        m_bottomRight = {topLeft.x + size.width, topLeft.y + size.height};
        m_posH = 0;
        m_posV = 0;
        m_shownH = 0;
        m_shownV = 0;
        m_segment = segmentLength;
        m_thickness = thickness;
        m_width = size.width;
        m_height = size.height;
    }

    void FrameEffect::draw(gfx::Canvas& canvas) {
        if (!m_picture.isValid()) {
            return;
        }
        constexpr int kRestart = 0;
        const bool step = canvas.isAnimationFrame();
        if (step) {
            m_shownH = m_posH;
            m_shownV = m_posV;
        }
        int posH = m_shownH;
        int posV = m_shownV;
        const int overH = posH - m_width;
        const int overV = posV - m_width;
        ++posH;
        ++posV;
        core::Rect horizontal;
        // the horizontal piece is cut from the picture at x 3, y 0
        if (overH > 0) {
            horizontal = {3, 0, m_segment - std::abs(overH), m_thickness};
        } else if (posH >= m_segment) {
            horizontal = {3, 0, m_segment, m_thickness};
        } else {
            horizontal = {3, 0, posH, m_thickness};
        }
        const auto blit = [&](core::Point at, const core::Rect& source) {
            if (source.width > 0 && source.height > 0) {
                canvas.drawPicture(m_picture, at, source);
            }
        };
        if (posH <= m_segment) {
            blit(m_topLeft, horizontal);
        } else {
            blit({posH - m_segment + m_topLeft.x - 1, m_topLeft.y}, horizontal);
        }
        if (overH > 0) {
            blit({m_topLeft.x, m_bottomRight.y - m_thickness}, horizontal);
        } else {
            blit({m_bottomRight.x - posH, m_bottomRight.y - m_thickness}, horizontal);
        }
        core::Rect vertical;
        // the vertical piece is cut from the picture at x 0, y 3
        if (m_height - m_thickness < overV) {
            vertical = {0, 3, m_thickness, m_segment - std::abs(overV - m_height + m_thickness)};
        } else if (m_segment + m_width <= posV) {
            vertical = {0, 3, m_thickness, m_segment};
        } else {
            vertical = {0, 3, m_thickness, overH};
        }
        if (overV <= m_segment) {
            blit({m_bottomRight.x - m_thickness, m_topLeft.y + m_thickness}, vertical);
        } else {
            blit({m_bottomRight.x - m_thickness, m_topLeft.y + m_thickness + (overV - m_segment)}, vertical);
        }
        if (m_height - m_thickness <= overV) {
            blit(m_topLeft, vertical);
        } else {
            blit({m_topLeft.x, m_bottomRight.y - m_thickness - overV}, vertical);
        }
        if (m_height - m_thickness <= overH) {
            posH = kRestart;
        }
        if (m_height - m_thickness + m_segment <= overV) {
            posV = posH;
        }
        if (step) {
            m_posH = posH;
            m_posV = posV;
        }
    }

}
