#include "gfx/PictureLibrary.h"

#include "core/FileSystem.h"
#include "core/Log.h"
#include "core/StringUtil.h"
#include "formats/BmpDecoder.h"
#include "formats/GamePixels.h"
#include "formats/JpegDecoder.h"
#include "formats/PngCodec.h"
#include "gfx/RenderDevice.h"

#include <algorithm>
#include <cmath>
#include <system_error>

namespace fs = std::filesystem;

namespace gfx {

    namespace {

        /// @brief Shrinks an HD picture to the logical size (each block averaged, colours weighed by alpha, a block with any pixel seen is seen).
        /// @param image The HD pixels.
        /// @param detail How many pixels across make one logical pixel.
        /// @return The pixels at the logical size.
        formats::Image shrink(const formats::Image& image, int detail) {
            formats::Image small(image.width() / detail, image.height() / detail);
            for (int y = 0; y < small.height(); ++y) {
                for (int x = 0; x < small.width(); ++x) {
                    int red = 0;
                    int green = 0;
                    int blue = 0;
                    int alpha = 0;
                    for (int dy = 0; dy < detail; ++dy) {
                        for (int dx = 0; dx < detail; ++dx) {
                            const core::Color& c = image.at(x * detail + dx, y * detail + dy);
                            red += c.r * c.a;
                            green += c.g * c.a;
                            blue += c.b * c.a;
                            alpha += c.a;
                        }
                    }
                    if (alpha == 0) {
                        small.at(x, y) = {0, 0, 0, 0};
                        continue;
                    }
                    const int count = detail * detail;
                    small.at(x, y) = {
                        static_cast<std::uint8_t>((red + alpha / 2) / alpha),
                        static_cast<std::uint8_t>((green + alpha / 2) / alpha),
                        static_cast<std::uint8_t>((blue + alpha / 2) / alpha),
                        static_cast<std::uint8_t>(std::max(1, (alpha + count / 2) / count))
                    };
                }
            }
            return small;
        }

    }

    void PictureLibrary::addFile(const fs::path& bitmapFile) { m_files.emplace(core::toLower(bitmapFile.stem().string()), bitmapFile); }

    void PictureLibrary::addReplacement(const std::string& name, const fs::path& file, int detail) {
        m_replacements[core::toLower(name)] = {file, std::max(detail, 1)};
        m_missing.clear();
    }

    std::optional<PictureLibrary::Pixels> PictureLibrary::replacement(
        const std::string& name,
        const std::optional<formats::Image>& original,
        core::Color tint
    ) {
        const auto found = m_replacements.find(core::toLower(name));
        if (found == m_replacements.end()) {
            return std::nullopt;
        }
        const fs::path& file = found->second.file;
        const int detail = found->second.detail;
        const auto data = core::readFile(file);
        const std::string extension = core::toLower(file.extension().string());
        std::optional<formats::Image> image;
        if (data) {
            image = extension == ".png" ? formats::decodePng(*data) : extension == ".jpg" ? formats::decodeJpeg(*data) : formats::decodeBmp(*data);
        }
        if (!image) {
            core::logWarning("cannot read the replacement picture ", file.string());
            m_replacements.erase(found);
            return std::nullopt;
        }
        if (original && (original->width() * detail != image->width() || original->height() * detail != image->height())) {
            core::logWarning(
                "the replacement picture ",
                file.string(),
                " is ",
                image->width(),
                "x",
                image->height(),
                ", not ",
                original->width() * detail,
                "x",
                original->height() * detail,
                " as ",
                name,
                detail > 1 ? " at " + std::to_string(detail) + "x" : std::string(),
                ": the original is used"
            );
            m_replacements.erase(found);
            return std::nullopt;
        }
        if (image->width() % detail != 0 || image->height() % detail != 0) {
            core::logWarning(
                "the replacement picture ",
                file.string(),
                " is ",
                image->width(),
                "x",
                image->height(),
                ", which is not ",
                detail,
                "x a size: it is not used"
            );
            m_replacements.erase(found);
            return std::nullopt;
        }
        if (extension == ".bmp") {
            formats::applyPictureRules(*image, tint);
        } else {
            formats::addColor(*image, tint);
        }
        return Pixels{std::move(*image), detail};
    }

    void PictureLibrary::addDirectory(const fs::path& directory) {
        std::error_code error;
        for (const auto& entry : fs::directory_iterator(directory, error)) {
            if (entry.is_regular_file(error) && core::toLower(entry.path().extension().string()) == ".bmp") {
                addFile(entry.path());
            }
        }
        if (error) {
            core::logWarning("cannot read the picture directory ", directory.string());
        }
    }

    bool PictureLibrary::addArchive(const fs::path& archiveFile) {
        auto archive = formats::MrgArchive::open(archiveFile);
        if (!archive || archive->kind() == formats::MrgArchive::Kind::Targa) {
            core::logWarning("cannot open the picture archive ", archiveFile.string());
            return false;
        }
        m_archives.push_back(std::move(*archive));
        m_missing.clear();
        return true;
    }

    std::optional<PictureId> PictureLibrary::store(const std::string& key, const Pixels& pixels) {
        auto texture = Texture::create(m_device, pixels.image, pixels.detail);
        if (!texture) {
            m_missing.insert(key);
            return std::nullopt;
        }
        m_pictures.emplace_back(std::move(*texture), pixels.logical ? *pixels.logical : pixels.detail > 1 ? shrink(pixels.image, pixels.detail) : pixels.image);
        const int id = static_cast<int>(m_pictures.size()) - 1;
        m_loaded.emplace(key, id);
        return PictureId{id};
    }

    std::optional<PictureId> PictureLibrary::find(const std::string& name, core::Color tint, Source source) {
        if (name.empty()) {
            return std::nullopt;
        }
        const bool tinted = tint.r != 0 || tint.g != 0 || tint.b != 0;
        const std::string sourceKey = source == Source::Any ? std::string() : source == Source::Bitmap ? "#bmg" : "#jmg";
        const std::string key = core::toLower(name) + sourceKey +
                                (tinted ? "#" + std::to_string(tint.r) + ',' + std::to_string(tint.g) + ',' + std::to_string(tint.b) : std::string());
        if (const auto loaded = m_loaded.find(key); loaded != m_loaded.end()) {
            return PictureId{loaded->second};
        }
        if (m_missing.count(key) != 0) {
            return std::nullopt;
        }
        const auto pixels = load(name, tint, source);
        if (!pixels) {
            core::logWarning("picture ", name, " does not exist");
            m_missing.insert(key);
            return std::nullopt;
        }
        return store(key, *pixels);
    }

    std::optional<PictureId> PictureLibrary::findStretched(const std::string& name, int width, int height) {
        const std::string key = core::toLower(name) + "#stretch" + std::to_string(width) + "x" + std::to_string(height);
        if (const auto loaded = m_loaded.find(key); loaded != m_loaded.end()) {
            return PictureId{loaded->second};
        }
        if (m_missing.count(key) != 0 || width <= 0 || height <= 0) {
            return std::nullopt;
        }
        const auto pixels = load(name, {0, 0, 0, 0}, Source::Any);
        if (!pixels || pixels->image.isEmpty()) {
            m_missing.insert(key);
            return std::nullopt;
        }
        const formats::Image& source = pixels->image;
        const int detail = pixels->detail;
        width *= detail;
        height *= detail;
        const auto texel = [&](int x, int y) {
            x = std::clamp(x, 0, source.width() - 1);
            y = std::clamp(y, 0, source.height() - 1);
            const core::Color& c = source.at(x, y);
            return c.a == 0 ? core::Color{0, 0, 0, 0} : c;
        };
        formats::Image image(width, height);
        const double scaleX = static_cast<double>(source.width()) / width;
        const double scaleY = static_cast<double>(source.height()) / height;
        for (int y = 0; y < height; ++y) {
            const double sy = (y + 0.5) * scaleY - 0.5;
            const int y0 = static_cast<int>(std::floor(sy));
            const double fy = sy - y0;
            for (int x = 0; x < width; ++x) {
                const double sx = (x + 0.5) * scaleX - 0.5;
                const int x0 = static_cast<int>(std::floor(sx));
                const double fx = sx - x0;
                const core::Color p00 = texel(x0, y0);
                const core::Color p10 = texel(x0 + 1, y0);
                const core::Color p01 = texel(x0, y0 + 1);
                const core::Color p11 = texel(x0 + 1, y0 + 1);
                const auto mix = [&](std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d) {
                    const double top = a + (b - a) * fx;
                    const double bottom = c + (d - c) * fx;
                    return static_cast<std::uint8_t>(std::lround(top + (bottom - top) * fy));
                };
                core::Color out{mix(p00.r, p10.r, p01.r, p11.r), mix(p00.g, p10.g, p01.g, p11.g), mix(p00.b, p10.b, p01.b, p11.b), 255}; // opaque alpha
                // RGB565 precision: 5 bits of red and blue (shift 3), 6 bits of green (shift 2)
                if ((out.r >> 3) == 0 && (out.g >> 2) == 0 && (out.b >> 3) == 0) {
                    out.a = 0;
                }
                image.at(x, y) = out;
            }
        }
        formats::quantizeTo16Bit(image);
        return store(key, Pixels{std::move(image), detail});
    }

    std::optional<formats::Image> PictureLibrary::decode(const std::string& name, core::Color tint, Source source) {
        auto pixels = load(name, tint, source);
        if (!pixels) {
            return std::nullopt;
        }
        if (pixels->logical) {
            return std::move(*pixels->logical);
        }
        return pixels->detail > 1 ? shrink(pixels->image, pixels->detail) : std::move(pixels->image);
    }

    PictureLibrary::Pixels PictureLibrary::scaled(formats::Image image, PixelArtPart part) const {
        const int detail = m_device.pixelArtDetail(part);
        if (detail <= 1) {
            return Pixels{std::move(image)};
        }
        formats::Image big = formats::scalePixelArt(image, m_device.pixelArtFilter(), detail);
        return Pixels{std::move(big), detail, std::move(image)};
    }

    std::optional<PictureLibrary::Pixels> PictureLibrary::load(const std::string& name, core::Color tint, Source source) {
        std::optional<core::Bytes> data;
        bool jpeg = false;
        if (const auto file = m_files.find(core::toLower(name)); source != Source::Jpeg && file != m_files.end()) {
            data = core::readFile(file->second);
        }
        for (auto archive = m_archives.rbegin(); !data && archive != m_archives.rend(); ++archive) {
            const bool isJpeg = archive->kind() == formats::MrgArchive::Kind::Jpeg;
            if ((source == Source::Bitmap && isJpeg) || (source == Source::Jpeg && !isJpeg)) {
                continue;
            }
            data = archive->read(name);
            jpeg = data && isJpeg;
        }
        std::optional<formats::Image> image;
        if (data) {
            image = jpeg ? formats::decodeJpeg(*data) : formats::decodeBmp(*data);
        }
        if (!m_replacements.empty()) {
            if (auto replaced = replacement(name, image, tint)) {
                return replaced;
            }
        }
        if (!image) {
            return std::nullopt;
        }
        if (jpeg) {
            formats::addColor(*image, tint);
            formats::quantizeTo16Bit(*image);
        } else {
            formats::applyPictureRules(*image, tint);
            const bool digits = core::toLower(name).rfind("num_", 0) == 0;
            return scaled(std::move(*image), digits ? PixelArtPart::Text : PixelArtPart::Pictures);
        }
        return Pixels{std::move(*image)};
    }

    std::optional<PictureId> PictureLibrary::loadFile(const fs::path& file) {
        const std::string key = "file:" + core::toLower(file.generic_string());
        if (const auto loaded = m_loaded.find(key); loaded != m_loaded.end()) {
            return PictureId{loaded->second};
        }
        if (m_missing.count(key) != 0) {
            return std::nullopt;
        }
        const auto data = core::readFile(file);
        std::optional<formats::Image> image;
        const bool jpeg = core::toLower(file.extension().string()) == ".jpg";
        if (data) {
            if (jpeg) {
                image = formats::decodeJpeg(*data);
                if (image) {
                    formats::quantizeTo16Bit(*image);
                }
            } else {
                image = formats::decodeBmp(*data);
                if (image) {
                    formats::applyPictureRules(*image);
                }
            }
        }
        if (!m_replacements.empty()) {
            if (auto replaced = replacement(file.stem().string(), image, {0, 0, 0, 0})) {
                return store(key, *replaced);
            }
        }
        if (!image) {
            core::logWarning("cannot load the picture ", file.string());
            m_missing.insert(key);
            return std::nullopt;
        }
        return store(key, jpeg ? Pixels{std::move(*image)} : scaled(std::move(*image)));
    }

}
