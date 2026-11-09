#include "ui/TiledFrame.h"

namespace ui {

    void TiledFrame::tileRow(
        gfx::Canvas& canvas,
        int sourceTop,
        int sourceHeight,
        int destY,
        const core::Rect& area
    ) const {
        if (sourceHeight <= 0) {
            return;
        }
        const int middleSource = m_source.width - m_left - m_right;
        canvas.drawPicture(m_picture, {area.x, destY}, core::Rect{m_source.x, sourceTop, m_left, sourceHeight});
        canvas.drawPicture(m_picture, {area.right() - m_right, destY}, core::Rect{m_source.x + m_source.width - m_right, sourceTop, m_right, sourceHeight});
        if (middleSource <= 0) {
            return;
        }
        const int middle = area.width - m_left - m_right;
        for (int x = 0; x < middle; x += middleSource) {
            const int width = middle - x < middleSource ? middle - x : middleSource;
            canvas.drawPicture(m_picture, {area.x + m_left + x, destY}, core::Rect{m_source.x + m_left, sourceTop, width, sourceHeight});
        }
    }

    void TiledFrame::draw(gfx::Canvas& canvas, const core::Rect& area, int state) const {
        if (!m_picture.isValid()) {
            return;
        }
        const int stateTop = m_source.y + state * m_source.height;
        tileRow(canvas, stateTop, m_top, area.y, area);
        tileRow(canvas, stateTop + m_source.height - m_bottom, m_bottom, area.bottom() - m_bottom, area);
        const int middleSource = m_source.height - m_top - m_bottom;
        if (middleSource <= 0) {
            return;
        }
        const int middle = area.height - m_top - m_bottom;
        for (int y = 0; y < middle; y += middleSource) {
            const int height = middle - y < middleSource ? middle - y : middleSource;
            tileRow(canvas, stateTop + m_top, height, area.y + m_top + y, area);
        }
    }

    bool TiledFrame::isOpaqueAt(const gfx::Picture& picture, core::Point local, int width, int height) const {
        const auto map = [](int p, int size, int sourceSize, int low, int high) {
            if (p <= low) {
                return p;
            }
            if (p <= size - high) {
                const int middle = sourceSize - low - high;
                return middle > 0 ? (p - low) % middle + low : low;
            }
            return p - (size - high) + sourceSize - high;
        };
        const int x = map(local.x, width, m_source.width, m_left, m_right);
        const int y = map(local.y, height, m_source.height, m_top, m_bottom);
        return picture.isOpaque(m_source.x + x, m_source.y + y);
    }

}
