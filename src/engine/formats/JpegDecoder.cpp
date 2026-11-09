#include "formats/JpegDecoder.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb_image.h"

namespace formats {

    std::optional<Image> decodeJpeg(const core::Bytes& data) {
        int width = 0;
        int height = 0;
        int channels = 0;
        stbi_uc* pixels = stbi_load_from_memory(data.data(), static_cast<int>(data.size()), &width, &height, &channels, 3); // request 3 channels (RGB)
        if (pixels == nullptr) {
            return std::nullopt;
        }
        Image image(width, height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const stbi_uc* p =
                    pixels + (static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)) * 3; // 3 bytes per RGB pixel
                image.at(x, y) = {p[0], p[1], p[2], 255}; // opaque alpha
            }
        }
        stbi_image_free(pixels);
        return image;
    }

}
