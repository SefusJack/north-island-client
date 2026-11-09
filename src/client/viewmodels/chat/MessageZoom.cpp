#include "client/viewmodels/chat/MessageZoom.h"

namespace client {

    MessageZoom::MessageZoom(
        const Shape& shape,
        std::uint64_t startMs
    )
        : m_shape(shape),
          m_startMs(startMs) {}

    core::Rect MessageZoom::advance(std::uint64_t nowMs, const core::Rect& target) {
        const int colStepPx = m_shape.longestLineWidth / (kColSteps - 1);
        const int rowStepPx = m_shape.textHeight / (kRowSteps - 1);
        const int thinHeight = m_shape.marginTop + m_shape.marginBottom + 6; // height of the thin bar beyond the margins
        int left = target.x;
        int top = target.y;
        if (m_colStep < kColSteps) {
            m_width = m_colStep * colStepPx + m_shape.marginLeft + m_shape.marginRight + 20; // width of the box at step 0 beyond the margins
            left = (target.width - m_width) / 2 + target.x;
            m_height = thinHeight;
            top = (target.height - m_height) / 2 + target.y;
        } else {
            m_width = target.width;
            if (m_width % 2 == 1) {
                ++m_width;
            }
            if (m_height % 2 == 1) {
                ++m_height;
            }
        }
        if (m_colStep == kColSteps) {
            if (m_rowStep < kRowSteps) {
                m_height = m_rowStep * rowStepPx + thinHeight;
                top = (target.height - m_height) / 2 + target.y;
            } else {
                m_height = target.height;
                top = target.y;
            }
        } else if (m_colStep > kColSteps) {
            top = (target.height - m_height) / 2 + target.y;
        }
        if (m_colStep == kColSteps && m_rowStep == kRowSteps) {
            m_textVisible = true;
        } else {
            const int elapsed = static_cast<int>(nowMs - m_startMs);
            m_colStep = m_colStep < kColSteps ? elapsed : kColSteps;
            m_rowStep = m_colStep == kColSteps && m_rowStep < kRowSteps ? elapsed : kRowSteps;
        }
        return {left, top, m_width, m_height};
    }

}
