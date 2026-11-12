#include "gfx/Picture.h"

#include "formats/GamePixels.h"

#include <algorithm>
#include <utility>

namespace gfx {

    Picture::Picture(
        Texture texture,
        const formats::Image& image
    )
        : Picture(std::move(texture), Mask::of(image)) {}

    Picture::Picture(
        Texture texture,
        Mask mask
    )
        : m_texture(std::move(texture)),
          m_opaque(std::move(mask.bits)),
          m_topLeft(mask.topLeft) {}

    Picture::Mask Picture::Mask::of(const formats::Image& image) {
        Mask mask;
        if (!image.isEmpty()) {
            mask.topLeft = image.at(0, 0);
        }
        mask.bits = formats::opaqueBits(image);
        return mask;
    }

    bool Picture::isOpaque(int x, int y) const {
        if (x < 0 || y < 0 || x >= width() || y >= height()) {
            return false;
        }
        const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(width()) + static_cast<std::size_t>(x);
        return index / 64 < m_opaque.size() && ((m_opaque[index / 64] >> (index % 64)) & 1) != 0;
    }

}
