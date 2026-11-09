#include "gfx/Picture.h"

#include <utility>

namespace gfx {

    Picture::Picture(
        Texture texture,
        const formats::Image& image
    )
        : m_texture(std::move(texture)) {
        if (!image.isEmpty()) {
            m_topLeft = image.at(0, 0);
        }
        m_opaque.reserve(image.pixels().size());
        for (const core::Color& pixel : image.pixels()) {
            m_opaque.push_back(pixel.a != 0);
        }
    }

    bool Picture::isOpaque(int x, int y) const {
        if (x < 0 || y < 0 || x >= width() || y >= height()) {
            return false;
        }
        return m_opaque[static_cast<std::size_t>(y) * static_cast<std::size_t>(width()) + static_cast<std::size_t>(x)];
    }

}
