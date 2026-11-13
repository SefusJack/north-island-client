#include "gfx/Picture.h"

#include "formats/GamePixels.h"
#include "gfx/PictureLibrary.h"

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

    Picture Picture::deferred(int width, int height, const PictureLibrary* owner, int id) {
        Picture picture;
        picture.m_owner = owner;
        picture.m_id = id;
        picture.m_width = width;
        picture.m_height = height;
        return picture;
    }

    Picture Picture::inParts(Texture texture, const PictureLibrary* owner, int id) {
        Picture picture;
        picture.m_partsOwner = owner;
        picture.m_id = id;
        const std::size_t pixels = static_cast<std::size_t>(texture.width()) * static_cast<std::size_t>(texture.height());
        picture.m_opaque.assign((pixels + 63) / 64, ~std::uint64_t{0}); // a JPEG: every pixel is opaque
        if (pixels % 64 != 0) {
            picture.m_opaque.back() = (std::uint64_t{1} << (pixels % 64)) - 1; // as opaqueBits leaves the bits past the end clear
        }
        picture.m_texture = std::move(texture);
        return picture;
    }

    void Picture::needRows(int top, int bottom) const {
        // the library marks this very slot done (partsDone) once every row is there
        const_cast<PictureLibrary*>(m_partsOwner)->needRows(m_id, top, bottom);
    }

    void Picture::load() const {
        if (m_partsOwner != nullptr) {
            needRows(0, m_texture.height()); // a use that may show any row: decode what is left
            return;
        }
        if (m_owner == nullptr) {
            return;
        }
        // the library puts the loaded picture into this very slot, so the members read after this call are the loaded ones
        const_cast<PictureLibrary*>(m_owner)->loadDeferred(m_id);
    }

    bool Picture::isOpaque(int x, int y) const {
        load();
        if (x < 0 || y < 0 || x >= width() || y >= height()) {
            return false;
        }
        const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(width()) + static_cast<std::size_t>(x);
        return index / 64 < m_opaque.size() && ((m_opaque[index / 64] >> (index % 64)) & 1) != 0;
    }

}
