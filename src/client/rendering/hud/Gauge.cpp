#include "client/rendering/hud/Gauge.h"

#include "client/rendering/hud/GaugeWipe.h"

namespace client {

    Gauge::Gauge(
        gfx::PictureLibrary& pictures,
        const std::string& picture
    )
        : m_picture(pictures.decodeDetailed(picture)) {}

    void Gauge::paint(gfx::Canvas& canvas, core::Point at, long long value, long long maximum) {
        if (!m_picture) {
            return;
        }
        const int percent = gaugePercent(value, maximum);
        if (percent != m_percent) {
            m_percent = percent;
            m_texture = gfx::Texture::create(canvas.device(), wipeGauge(m_picture->image, percent, m_picture->detail), m_picture->detail);
        }
        if (m_texture) {
            canvas.drawTexture(*m_texture, at);
        }
    }

}
