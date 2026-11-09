#include "client/subsystems/battle/SkillZoom.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        /// @brief Delphi's Round: to the nearest whole number, halves to the even one.
        /// @param value The value.
        /// @return The rounded value.
        int delphiRound(double value) { return static_cast<int>(std::nearbyint(value)); }

    }

    bool SkillZoom::decide(int span, int position, int targetPosition) {
        if (position - targetPosition >= span || span <= 0) {
            reset();
            return false;
        }
        const int closeness = span - std::abs(position - targetPosition);
        m_scale = static_cast<float>(static_cast<double>(closeness) / static_cast<double>(span));
        return true;
    }

    void SkillZoom::apply(int casterY, int nearBottom) {
        const int insetX = delphiRound(kInsetX * static_cast<double>(m_scale));
        const int insetY = delphiRound(kInsetY * static_cast<double>(m_scale));
        m_left = insetX;
        m_top = insetY;
        m_right = kScreenWidth - insetX;
        m_bottom = kScreenHeight - insetY;
        m_on = true;
        if (casterY + kViewHalfHeight > nearBottom) {
            if (m_top > 0) {
                m_top += nearBottom - casterY;
            }
            if (m_bottom < kScreenHeight) {
                m_bottom += nearBottom - casterY;
            }
            keepOnScreen();
        }
        keepOnScreen();
    }

    void SkillZoom::reset() {
        m_left = 0;
        m_top = 0;
        m_right = kScreenWidth;
        m_bottom = kScreenHeight;
        m_on = false;
        m_scale = 0.0f;
    }

    void SkillZoom::keepOnScreen() {
        if (m_left < 0) {
            m_right -= m_left;
            m_left = 0;
        }
        if (m_right > kScreenWidth) {
            m_left -= m_right - kScreenWidth;
            m_right = kScreenWidth;
        }
        if (m_top < 0) {
            m_bottom -= m_top;
            m_top = 0;
        }
        if (m_bottom > kScreenHeight) {
            m_top -= m_bottom - kScreenHeight;
            m_bottom = kScreenHeight;
        }
    }

    std::optional<core::Rect> SkillZoom::part() const {
        if (!m_on) {
            return std::nullopt;
        }
        const int left = std::max(m_left, 0);
        const int top = std::max(m_top, 0);
        const int right = std::min(m_right, kScreenWidth);
        const int bottom = std::min(m_bottom, kScreenHeight);
        if (right <= left || bottom <= top || (left == 0 && top == 0 && right == kScreenWidth && bottom == kScreenHeight)) {
            return std::nullopt;
        }
        return core::Rect{left, top, right - left, bottom - top};
    }

    core::Rect SkillZoom::sourceIn(const core::Rect& part, const core::Rect& window) {
        const double scaleX = static_cast<double>(part.width) / kScreenWidth;
        const double scaleY = static_cast<double>(part.height) / kScreenHeight;
        return {
            part.x + static_cast<int>(std::lround(window.x * scaleX)),
            part.y + static_cast<int>(std::lround(window.y * scaleY)),
            static_cast<int>(std::lround(window.width * scaleX)),
            static_cast<int>(std::lround(window.height * scaleY))
        };
    }

}
