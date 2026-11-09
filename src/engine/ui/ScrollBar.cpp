#include "ui/ScrollBar.h"

#include <algorithm>

namespace ui {

    namespace {

        constexpr std::uint64_t kRepeatMs = 100; // auto-repeat interval while the mouse is held on the track, ms

    }

    ScrollBar::ScrollBar() {
        declareEvent("Scroll");
        setCursor(CursorKind::Hand);
    }

    void ScrollBar::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Track") {
            m_trackName = value;
        } else if (name == "Thumb") {
            m_thumbName = value;
        } else if (name == "ThumbSource") {
            m_thumbSource = LoadContext::parseRect(value);
        } else if (name == "ThumbCap") {
            m_thumbCap = LoadContext::parseInt(value);
        } else if (name == "TrackCap") {
            m_trackCap = LoadContext::parseInt(value);
        } else if (name == "ThumbX") {
            m_thumbX = LoadContext::parseInt(value);
        } else if (name == "ThumbInset") {
            m_thumbInset = LoadContext::parseInt(value);
        } else if (name == "MinThumb") {
            m_minThumb = LoadContext::parseInt(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    void ScrollBar::finishLoading(const LoadContext& context) {
        m_track = context.picture(m_trackName);
        m_thumb = context.picture(m_thumbName);
        if (m_thumb.isValid() && !m_thumbSource) {
            const gfx::Picture& thumb = context.pictureData(m_thumb);
            m_thumbSource = core::Rect{0, 0, thumb.width() / 3, thumb.height()}; // default: one third of the picture width is one thumb state
        }
        if (!m_thumbX && m_thumbSource) {
            const int trackWidth = m_track.isValid() ? context.pictureData(m_track).width() : bounds().width;
            m_thumbX = (trackWidth - m_thumbSource->width) / 2;
        }
        setRange(0, 0);
        Element::finishLoading(context);
    }

    bool ScrollBar::hitTest(core::Point local, const gfx::PictureLibrary& pictures) const {
        (void)pictures;
        return core::Rect{0, 0, bounds().width, bounds().height}.contains(local);
    }

    void ScrollBar::setRange(int count, int page) {
        m_page = page;
        m_maximum = std::max(count, page);
        if (m_maximum != 0) {
            m_thumbSize = static_cast<int>(static_cast<double>(m_page) / m_maximum * bounds().height);
        }
        m_thumbSize = std::max({m_thumbSize, m_thumbCap * 2, m_minThumb});
        m_trackLength = bounds().height - m_thumbSize;
        if (m_position + m_page > m_maximum) {
            setPosition(m_maximum - m_page);
        }
        placeThumb();
    }

    void ScrollBar::setPosition(int position) {
        m_position = std::clamp(position, 0, std::max(0, m_maximum - m_page));
        placeThumb();
    }

    void ScrollBar::placeThumb() {
        if (m_maximum == m_page) {
            m_thumbPos = 0;
            return;
        }
        m_thumbPos = static_cast<int>(static_cast<double>(m_position) / (m_maximum - m_page) * (bounds().height - m_thumbSize));
    }

    void ScrollBar::lineUp() { setPosition(m_position - 1); }

    void ScrollBar::lineDown() { setPosition(m_position + 1); }

    void ScrollBar::pageUp() { setPosition(m_position - m_page); }

    void ScrollBar::pageDown() { setPosition(m_position + m_page); }

    void ScrollBar::stopTracking() {
        m_dragging = false;
        m_pagingUp = false;
        m_pagingDown = false;
    }

    void ScrollBar::onMouseDown(UiContext& context, MouseButton button, core::Point local) {
        (void)context;
        if (button != MouseButton::Left) {
            return;
        }
        if (isOnThumb(local)) {
            m_dragging = true;
            m_dragStartY = local.y;
            m_dragStartThumb = m_thumbPos;
            return;
        }
        m_pagingUp = local.y < m_thumbPos;
        m_pagingDown = local.y > m_thumbPos + m_thumbSize;
        m_lastRepeatMs = 0;
    }

    void ScrollBar::onMouseUp(UiContext& context, MouseButton button, core::Point local) {
        (void)context;
        (void)button;
        (void)local;
        stopTracking();
    }

    void ScrollBar::onCaptureReleased(UiContext& context) {
        (void)context;
        stopTracking();
    }

    void ScrollBar::onMouseDrag(UiContext& context, core::Point local) {
        m_mouseOverThumb = isOnThumb(local);
        if (!m_dragging || m_maximum == m_page || m_trackLength <= 0) {
            return;
        }
        m_thumbPos = std::clamp(local.y - m_dragStartY + m_dragStartThumb, 0, m_trackLength);
        const int position = static_cast<int>(static_cast<double>(m_thumbPos) / m_trackLength * (m_maximum - m_page));
        if (position != m_position) {
            m_position = position;
            raise(context, "Scroll", m_position);
        }
    }

    void ScrollBar::onMouseWheel(UiContext& context, int steps) {
        if (m_maximum == m_page) {
            return;
        }
        const int before = m_position;
        for (int i = 0; i < std::abs(steps); ++i) {
            steps > 0 ? lineUp() : lineDown();
        }
        if (m_position != before) {
            raise(context, "Scroll", m_position);
        }
    }

    void ScrollBar::update(UiContext& context) {
        if ((m_pagingUp || m_pagingDown) && m_hovered && !m_mouseOverThumb && context.nowMs >= m_lastRepeatMs + kRepeatMs) {
            const int before = m_position;
            m_pagingDown ? pageDown() : pageUp();
            m_lastRepeatMs = context.nowMs;
            if (m_position != before) {
                raise(context, "Scroll", m_position);
            }
        }
        Element::update(context);
    }

    void ScrollBar::draw(DrawContext& context, core::Point screenPosition) {
        m_hovered = context.hoveredId == id();
        const core::Point local = context.mouse - screenPosition;
        m_mouseOverThumb = m_hovered && isOnThumb(local);
        if (!m_hovered) {
            m_pagingUp = false;
            m_pagingDown = false;
        }
        gfx::Canvas& canvas = context.canvas;
        const int height = bounds().height;

        if (m_track.isValid()) {
            const gfx::Picture& track = canvas.pictures().picture(m_track);
            const int trackWidth = track.width();
            const int trackHeight = track.height();
            canvas.drawPicture(m_track, screenPosition, core::Rect{0, 0, trackWidth, m_trackCap});
            canvas.drawPicture(
                m_track,
                {screenPosition.x, screenPosition.y + height - m_trackCap},
                core::Rect{0, trackHeight - m_trackCap, trackWidth, m_trackCap}
            );
            const int middleSource = trackHeight - m_trackCap * 2;
            const int middle = height - m_trackCap * 2;
            for (int y = 0; middleSource > 0 && y < middle; y += middleSource) {
                const int part = std::min(middleSource, middle - y);
                canvas.drawPicture(m_track, {screenPosition.x, screenPosition.y + m_trackCap + y}, core::Rect{0, m_trackCap, trackWidth, part});
            }
        }

        if (!m_thumb.isValid() || !m_thumbSource) {
            return;
        }
        // thumb state: 2 dragged or paging, 1 mouse over, 0 normal
        const int state = m_dragging || m_pagingUp || m_pagingDown ? 2 : (m_mouseOverThumb ? 1 : 0);
        const core::Rect& source = *m_thumbSource;
        const int sourceX = source.x + state * source.width;
        const int x = screenPosition.x + m_thumbX.value_or(0);
        const int top = screenPosition.y + m_thumbPos;
        canvas.drawPicture(m_thumb, {x, top + m_thumbInset}, core::Rect{sourceX, source.y, source.width, m_thumbCap});
        const int middleSource = source.height - m_thumbCap * 2;
        const int middle = m_thumbSize - m_thumbCap * 2;
        for (int y = 0; middleSource > 0 && y < middle; y += middleSource) {
            const int part = std::min(middleSource, middle - y);
            canvas.drawPicture(m_thumb, {x, top + m_thumbCap + y}, core::Rect{sourceX, source.y + m_thumbCap, source.width, part});
        }
        canvas.drawPicture(
            m_thumb,
            {x, top + m_thumbSize - m_thumbCap - m_thumbInset},
            core::Rect{sourceX, source.y + source.height - m_thumbCap, source.width, m_thumbCap}
        );
    }

}
