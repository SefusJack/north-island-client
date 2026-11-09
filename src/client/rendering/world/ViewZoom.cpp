#include "client/rendering/world/ViewZoom.h"

#include "core/Between.h"

#include <algorithm>
#include <cmath>

namespace client {

    namespace {

        constexpr int kStepShare = 40; // one zoom level trims 1/40 of the window width/height from each side

    }

    core::Rect ViewZoom::part(int level) const {
        const int stepX = m_window.width / kStepShare;
        const int stepY = m_window.height / kStepShare;
        return {m_window.x + level * stepX, m_window.y + level * stepY, m_window.width - 2 * level * stepX, m_window.height - 2 * level * stepY};
    }

    core::Rect ViewZoom::shown(std::uint64_t nowMs) const {
        const core::Rect target = part(m_level);
        if (!m_gliding || nowMs >= m_fromMs + kGlideMs || nowMs < m_fromMs) {
            return target;
        }
        return core::between(m_from, target, static_cast<double>(nowMs - m_fromMs) / static_cast<double>(kGlideMs));
    }

    bool ViewZoom::isShownZoomed(std::uint64_t nowMs) const { return !(shown(nowMs) == m_window); }

    void ViewZoom::setWindow(const core::Rect& window) {
        if (window == m_window) {
            return;
        }
        m_window = window;
        reset();
    }

    void ViewZoom::glideFromHere() {
        m_from = shown(m_nowMs);
        m_fromMs = m_nowMs;
    }

    void ViewZoom::setLevel(int level) {
        level = std::clamp(level, m_outAllowed ? kMinLevel : 0, kMaxLevel);
        if (level != m_level) {
            glideFromHere();
            m_level = level;
        }
    }

    void ViewZoom::setOutAllowed(bool allowed) {
        m_outAllowed = allowed;
        if (!allowed && m_level < 0) {
            reset();
        }
    }

    void ViewZoom::zoomIn() { setLevel(m_level + 1); }

    void ViewZoom::zoomOut() { setLevel(m_level - 1); }

    void ViewZoom::zoomToMost() {
        if (m_level != kMaxLevel) {
            m_level = kMaxLevel;
            m_from = part(m_level);
        }
    }

    void ViewZoom::reset() {
        m_level = 0;
        m_from = m_window;
    }

    core::Point ViewZoom::toWindow(core::Point point, const core::Rect& part, const core::Rect& window) {
        if (part.width <= 0 || part.height <= 0) {
            return point;
        }
        return {
            window.x + static_cast<int>(std::lround(static_cast<double>(point.x - part.x) * window.width / part.width)),
            window.y + static_cast<int>(std::lround(static_cast<double>(point.y - part.y) * window.height / part.height))
        };
    }

    core::Point ViewZoom::fromWindow(core::Point point, const core::Rect& part, const core::Rect& window) {
        if (window.width <= 0 || window.height <= 0) {
            return point;
        }
        return {
            part.x + static_cast<int>(std::lround(static_cast<double>(point.x - window.x) * part.width / window.width)),
            part.y + static_cast<int>(std::lround(static_cast<double>(point.y - window.y) * part.height / window.height))
        };
    }

}
