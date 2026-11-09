#include "client/viewmodels/system/PercentSlider.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr std::uint64_t kRepeatMs = 100; // paging repeats one step every 100 ms while the mouse is held on the bar

    }

    void PercentSlider::placeThumb() { m_thumbPos = static_cast<int>(static_cast<double>(m_position) / kMaximum * kTrackLength); }

    void PercentSlider::setPosition(int position) {
        m_position = position;
        placeThumb();
    }

    int PercentSlider::thumbState(bool mouseOnThumb) const {
        if (mouseOnThumb) {
            return m_dragging ? 2 : 1; // thumb state: 2 dragged, 1 hovered
        }
        return m_pagingUp || m_pagingDown ? 2 : 0; // thumb state: 2 held by paging, 0 idle
    }

    void PercentSlider::press(int x) {
        if (onThumb(x)) {
            m_dragStartX = x;
            m_dragStartThumb = m_thumbPos;
            m_dragging = true;
            return;
        }
        m_dragging = false;
        m_dragStartX = 0;
        m_dragStartThumb = 0;
        m_pagingUp = x < m_thumbPos;
        m_pagingDown = x > m_thumbPos + kThumbSize;
    }

    void PercentSlider::drag(int x) {
        if (!m_dragging) {
            return;
        }
        m_thumbPos = std::clamp(x - m_dragStartX + m_dragStartThumb, 0, kTrackLength);
        m_position = static_cast<int>(static_cast<double>(m_thumbPos) / kTrackLength * kMaximum);
    }

    void PercentSlider::release() {
        m_dragging = false;
        m_dragStartX = 0;
        m_dragStartThumb = 0;
        m_pagingUp = false;
        m_pagingDown = false;
    }

    void PercentSlider::update(std::uint64_t nowMs, bool mouseOnBar, bool mouseOnThumb) {
        if (mouseOnBar && !mouseOnThumb) {
            if (m_lastRepeatMs + kRepeatMs < nowMs) {
                if (m_pagingDown) {
                    m_position = std::min(m_position + 1, kMaximum);
                    placeThumb();
                }
                if (m_pagingUp) {
                    m_position = std::max(m_position - 1, 0);
                    placeThumb();
                }
                m_lastRepeatMs = nowMs;
            }
        } else {
            m_pagingDown = false;
            m_pagingUp = false;
        }
    }

}
