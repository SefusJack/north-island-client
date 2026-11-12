#include "gfx/Texture.h"

#include "core/Log.h"
#include "gfx/RenderDevice.h"

namespace gfx {

    namespace {

        /// @brief Builds an SDL float rectangle from integers.
        /// @param x Left edge.
        /// @param y Top edge.
        /// @param width Width.
        /// @param height Height.
        /// @return The rectangle.
        SDL_FRect toFloat(int x, int y, int width, int height) {
            return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)};
        }

    }

    static_assert(sizeof(core::Color) == 4, "core::Color must be four bytes (r, g, b, a) to upload it");

    std::optional<Texture> Texture::create(const RenderDevice& device, const formats::Image& image, int detail) {
        if (image.isEmpty() || detail < 1 || image.width() < detail || image.height() < detail) {
            return std::nullopt;
        }
        const SDL_TextureAccess access = device.isAccelerated() ? SDL_TEXTUREACCESS_STATIC : SDL_TEXTUREACCESS_STREAMING;
        Texture texture;
        texture.m_texture.reset(SDL_CreateTexture(device.renderer(), SDL_PIXELFORMAT_RGBA32, access, image.width(), image.height()));
        if (!texture.m_texture) {
            core::logError("cannot create a ", image.width(), 'x', image.height(), " texture: ", SDL_GetError());
            return std::nullopt;
        }
        SDL_UpdateTexture(texture.m_texture.get(), nullptr, image.pixels().data(), image.width() * 4); // pitch: 4 bytes a pixel (RGBA)
        SDL_SetTextureBlendMode(texture.m_texture.get(), SDL_BLENDMODE_BLEND);
        if (detail > 1) {
            SDL_SetTextureScaleMode(texture.m_texture.get(), SDL_SCALEMODE_LINEAR);
        } else if (device.pixelScale() == 1 || !SDL_SetTextureScaleMode(texture.m_texture.get(), SDL_SCALEMODE_PIXELART)) {
            SDL_SetTextureScaleMode(texture.m_texture.get(), SDL_SCALEMODE_NEAREST);
        }
        texture.m_width = image.width() / detail;
        texture.m_height = image.height() / detail;
        texture.m_detail = detail;
        return texture;
    }

    void Texture::draw(const RenderDevice& device, core::Point destination, const std::optional<core::Rect>& source) const {
        const core::Rect area = source ? source->intersected({0, 0, m_width, m_height}) : core::Rect{0, 0, m_width, m_height};
        if (area.isEmpty()) {
            return;
        }
        const int shiftX = source ? area.x - source->x : 0;
        const int shiftY = source ? area.y - source->y : 0;
        drawScaled(device, {destination.x + shiftX, destination.y + shiftY, area.width, area.height}, area);
    }

    void Texture::drawQuads(const RenderDevice& device, const std::vector<Quad>& quads) const {
        if (!m_texture || quads.empty()) {
            return;
        }
        float textureWidth = 0.0f;
        float textureHeight = 0.0f;
        SDL_GetTextureSize(m_texture.get(), &textureWidth, &textureHeight);
        thread_local std::vector<SDL_Vertex> vertices;
        thread_local std::vector<int> indices;
        vertices.clear();
        indices.clear();
        for (const Quad& quad : quads) {
            // as draw: clip the source to the texture and move the destination with it
            const core::Rect area = quad.source.intersected({0, 0, m_width, m_height});
            if (area.isEmpty()) {
                continue;
            }
            const float minX = static_cast<float>(quad.destination.x + area.x - quad.source.x);
            const float minY = static_cast<float>(quad.destination.y + area.y - quad.source.y);
            const float maxX = minX + static_cast<float>(area.width);
            const float maxY = minY + static_cast<float>(area.height);
            // as SDL_RenderTexture: texture coordinates are source pixels over the texture size
            const float fromX = static_cast<float>(area.x * m_detail);
            const float fromY = static_cast<float>(area.y * m_detail);
            const float minU = fromX / textureWidth;
            const float minV = fromY / textureHeight;
            const float maxU = (fromX + static_cast<float>(area.width * m_detail)) / textureWidth;
            const float maxV = (fromY + static_cast<float>(area.height * m_detail)) / textureHeight;
            // as SDL_SetTextureColorMod / SDL_SetTextureAlphaMod: each channel over 255
            const SDL_FColor color{
                static_cast<float>(quad.color.r) / 255.0f,
                static_cast<float>(quad.color.g) / 255.0f,
                static_cast<float>(quad.color.b) / 255.0f,
                static_cast<float>(quad.color.a) / 255.0f
            };
            const int first = static_cast<int>(vertices.size());
            vertices.push_back({{minX, minY}, color, {minU, minV}});
            vertices.push_back({{maxX, minY}, color, {maxU, minV}});
            vertices.push_back({{maxX, maxY}, color, {maxU, maxV}});
            vertices.push_back({{minX, maxY}, color, {minU, maxV}});
            for (const int corner : {0, 1, 2, 0, 2, 3}) { // SDL's own corner order for a rectangle
                indices.push_back(first + corner);
            }
        }
        if (!vertices.empty()) {
            SDL_RenderGeometry(device.renderer(), m_texture.get(), vertices.data(), static_cast<int>(vertices.size()), indices.data(), static_cast<int>(indices.size()));
        }
    }

    void Texture::drawFlipped(
        const RenderDevice& device,
        core::Point destination,
        bool leftRight,
        bool topBottom,
        const std::optional<core::Rect>& source
    ) const {
        const core::Rect area = source ? source->intersected({0, 0, m_width, m_height}) : core::Rect{0, 0, m_width, m_height};
        if (!m_texture || area.isEmpty()) {
            return;
        }
        const SDL_FRect from = toFloat(area.x * m_detail, area.y * m_detail, area.width * m_detail, area.height * m_detail);
        const SDL_FRect to = toFloat(destination.x, destination.y, area.width, area.height);
        const auto flip = static_cast<SDL_FlipMode>((leftRight ? SDL_FLIP_HORIZONTAL : 0) | (topBottom ? SDL_FLIP_VERTICAL : 0));
        SDL_RenderTextureRotated(device.renderer(), m_texture.get(), &from, &to, 0.0, nullptr, flip);
    }

    void Texture::drawScaled(const RenderDevice& device, const core::Rect& destination, const std::optional<core::Rect>& source) const {
        if (!m_texture || destination.isEmpty()) {
            return;
        }
        const SDL_FRect to = toFloat(destination.x, destination.y, destination.width, destination.height);
        if (source) {
            const SDL_FRect from = toFloat(source->x * m_detail, source->y * m_detail, source->width * m_detail, source->height * m_detail);
            SDL_RenderTexture(device.renderer(), m_texture.get(), &from, &to);
        } else {
            SDL_RenderTexture(device.renderer(), m_texture.get(), nullptr, &to);
        }
    }

    void Texture::setColorMod(core::Color color) const { SDL_SetTextureColorMod(m_texture.get(), color.r, color.g, color.b); }

    void Texture::setAlphaMod(std::uint8_t alpha) const { SDL_SetTextureAlphaMod(m_texture.get(), alpha); }

    void Texture::setAdditive(bool additive) const { SDL_SetTextureBlendMode(m_texture.get(), additive ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND); }

}
