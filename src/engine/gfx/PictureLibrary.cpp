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
#include <chrono>
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

        /// @brief Scales a bitmap's pixels for the pixel-art detail (any thread).
        /// @param image The pixels, colour rules applied.
        /// @param detail Pixels across a logical pixel; 1 leaves them as they are.
        /// @param filter The pixel-art scaler.
        /// @return The pixels, with the logical-size ones when scaled.
        PictureLibrary::Pixels scaledWith(formats::Image image, int detail, formats::PixelArtFilter filter) {
            if (detail <= 1) {
                return PictureLibrary::Pixels{std::move(image)};
            }
            formats::Image big = formats::scalePixelArt(image, filter, detail);
            return PictureLibrary::Pixels{std::move(big), detail, std::move(image)};
        }

    }

    void PictureLibrary::addFile(const fs::path& bitmapFile) {
        m_prefetch.clear(); // workers read m_files: let them finish first
        m_files.emplace(core::toLower(bitmapFile.stem().string()), bitmapFile);
    }

    void PictureLibrary::addReplacement(const std::string& name, const fs::path& file, int detail) {
        m_replacements[core::toLower(name)] = {file, std::max(detail, 1)};
        m_missing.clear();
        m_prefetch.clear(); // read before the pack: find loads them again
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
        m_prefetch.clear(); // the new archive may hold newer pictures, and workers read m_archives: let them finish first
        m_archives.push_back(std::move(*archive));
        m_missing.clear();
        return true;
    }

    Picture PictureLibrary::makePicture(Texture texture, const Pixels& pixels) {
        // the logical-size pixels, without copying the image when it is already at that size
        if (pixels.logical) {
            return Picture(std::move(texture), *pixels.logical);
        }
        if (pixels.detail > 1) {
            return Picture(std::move(texture), shrink(pixels.image, pixels.detail));
        }
        return Picture(std::move(texture), pixels.image);
    }

    void PictureLibrary::collect(std::uint64_t nowMs) {
        m_clockMs = nowMs;
        constexpr std::uint64_t kLookEveryMs = 5000; // how often the large pictures are looked at, ms
        if (nowMs - m_lookedAtMs < kLookEveryMs) {
            return;
        }
        m_lookedAtMs = nowMs;
        for (auto& [id, recipe] : m_large) {
            const auto index = static_cast<std::size_t>(id);
            if (!recipe.freed && nowMs - m_lastUse[index] >= kFreeAfterMs) {
                core::logInfo("freed picture ", recipe.name, " (unused for ", (nowMs - m_lastUse[index]) / 1000, " s)");
                m_pictures[index] = Picture{}; // drops the texture and the pixel mask; the slot and its id stay
                recipe.freed = true;
                ++m_freedCount;
            }
        }
    }

    void PictureLibrary::reloadFreed(int id) {
        const auto found = m_large.find(id);
        if (found == m_large.end() || !found->second.freed) {
            return;
        }
        Recipe& recipe = found->second;
        const auto pixels = load(recipe.name, recipe.tint, recipe.source);
        auto texture = pixels ? Texture::create(m_device, pixels->image, pixels->detail) : std::nullopt;
        if (!texture) {
            core::logWarning("cannot load the freed picture ", recipe.name, " again");
            return; // stays freed: drawn as nothing
        }
        m_pictures[static_cast<std::size_t>(id)] = makePicture(std::move(*texture), *pixels);
        recipe.freed = false;
        --m_freedCount;
        core::logInfo("loaded the freed picture ", recipe.name, " again");
    }

    std::optional<PictureId> PictureLibrary::store(const std::string& key, const Pixels& pixels, const Recipe* recipe, std::optional<Picture::Mask> mask) {
        auto texture = Texture::create(m_device, pixels.image, pixels.detail);
        if (!texture) {
            m_missing.insert(key);
            return std::nullopt;
        }
        m_pictures.push_back(mask ? Picture(std::move(*texture), std::move(*mask)) : makePicture(std::move(*texture), pixels));
        m_lastUse.push_back(m_clockMs);
        const int id = static_cast<int>(m_pictures.size()) - 1;
        if (recipe && pixels.image.pixels().size() >= kLargePixels) {
            m_large.emplace(id, *recipe);
        }
        m_loaded.emplace(key, id);
        return PictureId{id};
    }

    std::string PictureLibrary::keyFor(const std::string& name, core::Color tint, Source source) {
        const bool tinted = tint.r != 0 || tint.g != 0 || tint.b != 0;
        const std::string sourceKey = source == Source::Any ? std::string() : source == Source::Bitmap ? "#bmg" : "#jmg";
        return core::toLower(name) + sourceKey +
               (tinted ? "#" + std::to_string(tint.r) + ',' + std::to_string(tint.g) + ',' + std::to_string(tint.b) : std::string());
    }

    void PictureLibrary::applyRules(formats::Image& image, bool jpeg, core::Color tint) {
        if (jpeg) {
            formats::addColor(image, tint);
            formats::quantizeTo16Bit(image);
        } else {
            formats::applyPictureRules(image, tint);
        }
    }

    std::optional<PictureId> PictureLibrary::findLoaded(const std::string& name, core::Color tint, Source source) const {
        if (const auto loaded = m_loaded.find(keyFor(name, tint, source)); loaded != m_loaded.end()) {
            return PictureId{loaded->second};
        }
        return std::nullopt;
    }

    std::optional<PictureLibrary::Raw> PictureLibrary::readRaw(const std::string& name, core::Color tint, Source source) {
        const std::string key = keyFor(name, tint, source);
        if (name.empty() || !m_replacements.empty() || m_loaded.count(key) != 0 || m_missing.count(key) != 0) {
            return std::nullopt; // find handles these: nothing to read, a pack picture, or already settled
        }
        Raw raw;
        raw.name = name;
        auto data = readData(name, source, raw.jpeg);
        if (!data) {
            return std::nullopt; // find reports the missing picture
        }
        raw.data = std::move(*data);
        return raw;
    }

    std::optional<formats::Image> PictureLibrary::decodeRaw(const Raw& raw, core::Color tint) {
        std::optional<formats::Image> image = raw.jpeg ? formats::decodeJpeg(raw.data) : formats::decodeBmp(raw.data);
        if (image) {
            applyRules(*image, raw.jpeg, tint);
        }
        return image;
    }

    std::optional<PictureId> PictureLibrary::storeDecoded(const Raw& raw, core::Color tint, Source source, std::optional<formats::Image> image) {
        const std::string key = keyFor(raw.name, tint, source);
        if (const auto loaded = m_loaded.find(key); loaded != m_loaded.end()) {
            return PictureId{loaded->second};
        }
        if (!image) {
            core::logWarning("picture ", raw.name, " does not exist");
            m_missing.insert(key);
            return std::nullopt;
        }
        const Recipe recipe{raw.name, tint, source};
        if (raw.jpeg) {
            return store(key, Pixels{std::move(*image)}, &recipe);
        }
        const bool digits = core::toLower(raw.name).rfind("num_", 0) == 0;
        return store(key, scaled(std::move(*image), digits ? PixelArtPart::Text : PixelArtPart::Pictures), &recipe);
    }

    std::optional<core::Bytes> PictureLibrary::readData(const std::string& name, Source source, bool& jpeg) {
        jpeg = false;
        if (const auto file = m_files.find(core::toLower(name)); source != Source::Jpeg && file != m_files.end()) {
            if (auto data = core::readFile(file->second)) {
                return data;
            }
        }
        const std::lock_guard<std::mutex> lock(m_archiveLock);
        for (auto archive = m_archives.rbegin(); archive != m_archives.rend(); ++archive) {
            const bool isJpeg = archive->kind() == formats::MrgArchive::Kind::Jpeg;
            if ((source == Source::Bitmap && isJpeg) || (source == Source::Jpeg && !isJpeg)) {
                continue;
            }
            if (auto data = archive->read(name)) {
                jpeg = isJpeg;
                return data;
            }
        }
        return std::nullopt;
    }

    bool PictureLibrary::hasData(const std::string& name, Source source) const {
        if (source != Source::Jpeg && m_files.count(core::toLower(name)) != 0) {
            return true;
        }
        for (const formats::MrgArchive& archive : m_archives) {
            const bool isJpeg = archive.kind() == formats::MrgArchive::Kind::Jpeg;
            if ((source == Source::Bitmap && isJpeg) || (source == Source::Jpeg && !isJpeg)) {
                continue;
            }
            if (archive.contains(name)) {
                return true;
            }
        }
        return false;
    }

    bool PictureLibrary::prefetch(const std::string& name, core::Color tint, Source source) {
        if (name.empty() || !m_replacements.empty()) {
            return false; // nothing to read, or a pack may replace it: find handles it
        }
        std::string key = keyFor(name, tint, source);
        if (m_prefetch.count(key) != 0 || m_loaded.count(key) != 0 || m_missing.count(key) != 0 || !hasData(name, source)) {
            return false; // started, loaded, or missing: find handles it
        }
        Prefetch prefetch;
        prefetch.tint = tint;
        prefetch.source = source;
        const bool digits = core::toLower(name).rfind("num_", 0) == 0;
        const int detail = m_device.pixelArtDetail(digits ? PixelArtPart::Text : PixelArtPart::Pictures);
        const formats::PixelArtFilter filter = m_device.pixelArtFilter();
        // the worker only reads m_files and m_archives (under m_archiveLock); addFile, addArchive and the destructor let it finish first.
        // It also does the pixel-art scaling and the pixel mask, so the frame that needs the picture only uploads it.
        prefetch.decoded = std::async(std::launch::async, [this, name, tint, source, detail, filter] {
            Decoded decoded;
            decoded.raw.name = name;
            if (auto data = readData(name, source, decoded.raw.jpeg)) {
                decoded.raw.data = std::move(*data);
                if (auto image = decodeRaw(decoded.raw, tint)) {
                    decoded.pixels = decoded.raw.jpeg ? Pixels{std::move(*image)} : scaledWith(std::move(*image), detail, filter);
                    const Pixels& pixels = *decoded.pixels;
                    decoded.mask = Picture::Mask::of(pixels.logical ? *pixels.logical : pixels.image);
                }
                decoded.raw.data = {};
            }
            return decoded;
        });
        m_prefetch.emplace(std::move(key), std::move(prefetch));
        return true;
    }

    std::optional<PictureId> PictureLibrary::settlePrefetch(std::unordered_map<std::string, Prefetch>::iterator prefetch) {
        Prefetch done = std::move(prefetch->second);
        m_prefetch.erase(prefetch);
        Decoded decoded = done.decoded.get();
        const std::string key = keyFor(decoded.raw.name, done.tint, done.source);
        if (const auto loaded = m_loaded.find(key); loaded != m_loaded.end()) {
            return PictureId{loaded->second};
        }
        if (!decoded.pixels) {
            core::logWarning("picture ", decoded.raw.name, " does not exist");
            m_missing.insert(key);
            return std::nullopt;
        }
        const Recipe recipe{decoded.raw.name, done.tint, done.source};
        return store(key, *decoded.pixels, &recipe, std::move(decoded.mask));
    }

    void PictureLibrary::pumpPrefetch(std::uint64_t nowMs) {
        if (m_pumpedAtMs == nowMs) {
            return; // this frame has had its upload (another party member's battle)
        }
        for (auto it = m_prefetch.begin(); it != m_prefetch.end(); ++it) {
            if (it->second.decoded.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                settlePrefetch(it);
                m_pumpedAtMs = nowMs;
                return; // one upload a frame
            }
        }
    }

    std::optional<PictureId> PictureLibrary::find(const std::string& name, core::Color tint, Source source) {
        if (name.empty()) {
            return std::nullopt;
        }
        const std::string key = keyFor(name, tint, source);
        if (const auto loaded = m_loaded.find(key); loaded != m_loaded.end()) {
            return PictureId{loaded->second};
        }
        if (m_missing.count(key) != 0) {
            return std::nullopt;
        }
        if (const auto prefetch = m_prefetch.find(key); prefetch != m_prefetch.end()) {
            return settlePrefetch(prefetch); // waits if the worker hasn't finished
        }
        const auto pixels = load(name, tint, source);
        if (!pixels) {
            core::logWarning("picture ", name, " does not exist");
            m_missing.insert(key);
            return std::nullopt;
        }
        const Recipe recipe{name, tint, source};
        return store(key, *pixels, &recipe);
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
        return scaledWith(std::move(image), m_device.pixelArtDetail(part), m_device.pixelArtFilter());
    }

    std::optional<PictureLibrary::Pixels> PictureLibrary::load(const std::string& name, core::Color tint, Source source) {
        bool jpeg = false;
        std::optional<core::Bytes> data = readData(name, source, jpeg);
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
        applyRules(*image, jpeg, tint);
        if (!jpeg) {
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
