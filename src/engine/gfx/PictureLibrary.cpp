#include "gfx/PictureLibrary.h"

#include "core/Clock.h"
#include "core/FileSystem.h"
#include "core/Log.h"
#include "core/StringUtil.h"
#include "core/WorkerPriority.h"
#include "formats/BmpDecoder.h"
#include "formats/GamePixels.h"
#include "formats/JpegDecoder.h"
#include "formats/PngCodec.h"
#include "gfx/RenderDevice.h"

#include <fstream>
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

    namespace {

        /// @brief The size of a picture file from its header, when it is a kind the decoders read.
        /// @param data The file's bytes.
        /// @param jpeg True for a JPEG, false for a Windows bitmap.
        /// @return The size, or nothing (the picture is then loaded at once, as before).
        std::optional<core::Size> headerSize(const core::Bytes& data, bool jpeg) {
            const auto u16 = [&](std::size_t at) { return static_cast<std::uint32_t>(data[at]) | (static_cast<std::uint32_t>(data[at + 1]) << 8); };
            const auto u32 = [&](std::size_t at) { return u16(at) | (u16(at + 2) << 16); };
            const auto be16 = [&](std::size_t at) { return (static_cast<int>(data[at]) << 8) | static_cast<int>(data[at + 1]); };
            if (!jpeg) {
                // BITMAPFILEHEADER (14 bytes) then BITMAPINFOHEADER: size, width, height, planes, bit count, compression
                if (data.size() < 54 || data[0] != 'B' || data[1] != 'M' || u32(14) < 40) {
                    return std::nullopt;
                }
                const auto width = static_cast<std::int32_t>(u32(18));
                const auto height = static_cast<std::int32_t>(u32(22));
                const std::uint32_t bits = u16(28);
                const std::uint32_t compression = u32(30);
                const bool known = compression == 0 || (compression == 3 && (bits == 16 || bits == 32));
                if (width <= 0 || height == 0 || !known || (bits != 1 && bits != 4 && bits != 8 && bits != 16 && bits != 24 && bits != 32)) {
                    return std::nullopt;
                }
                return core::Size{width, height < 0 ? -height : height};
            }
            if (data.size() < 4 || data[0] != 0xFF || data[1] != 0xD8) {
                return std::nullopt;
            }
            for (std::size_t at = 2; at + 9 < data.size();) {
                if (data[at] != 0xFF) {
                    return std::nullopt;
                }
                const std::uint8_t marker = data[at + 1];
                if (marker == 0xC0 || marker == 0xC2) { // baseline or progressive frame header: precision, height, width
                    const int height = be16(at + 5);
                    const int width = be16(at + 7);
                    return width > 0 && height > 0 ? std::optional<core::Size>(core::Size{width, height}) : std::nullopt;
                }
                if (marker == 0xD9 || marker == 0xDA || (marker >= 0xC1 && marker <= 0xCF && marker != 0xC4 && marker != 0xC8 && marker != 0xCC)) {
                    return std::nullopt; // end, scan data before a frame header, or a kind of JPEG the decoder does not read
                }
                at += 2 + static_cast<std::size_t>(be16(at + 2));
            }
            return std::nullopt;
        }

    }

    namespace {

        constexpr std::size_t kPartsFromBytes = 1000000; // JPEG files this large (the map backgrounds) may be decoded in parts

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
        if (!m_deferred.empty() && nowMs != m_warmedAtMs) {
            // a few ms of deferred pictures a call, so everything is loaded soon after the scene opened without holding it up
            m_warmedAtMs = nowMs;
            constexpr double kWarmBudgetMs = 6.0; // per call (a beat is 30 ms)
            const double startMs = core::nowPreciseMs();
            while (!m_deferred.empty() && core::nowPreciseMs() - startMs < kWarmBudgetMs) {
                const int id = m_deferred.begin()->first;
                m_pictures[static_cast<std::size_t>(id)].texture();
            }
        }
        if (!m_inParts.empty() && nowMs != m_partsWarmedAtMs) {
            // a few ms of a map background's remaining rows a call, nearest the rows last drawn first
            m_partsWarmedAtMs = nowMs;
            constexpr double kPartsBudgetMs = 6.0; // per call (a beat is 30 ms)
            constexpr int kBandsAtOnce = 2; // bands a decode
            const double startMs = core::nowPreciseMs();
            const int id = m_inParts.begin()->first;
            InParts& parts = m_inParts.begin()->second;
            const int count = parts.bands.count();
            while (parts.left > 0 && core::nowPreciseMs() - startMs < kPartsBudgetMs) {
                int next = -1;
                for (int away = 0; away < count && next < 0; ++away) {
                    const int below = parts.near + away;
                    const int above = parts.near - away;
                    if (below < count && below >= 0 && parts.done[static_cast<std::size_t>(below)] == 0) {
                        next = below;
                    } else if (above >= 0 && above < count && parts.done[static_cast<std::size_t>(above)] == 0) {
                        next = above;
                    }
                }
                if (next < 0) {
                    break;
                }
                decodeBands(id, parts, next, std::min(count, next + kBandsAtOnce));
            }
            if (parts.left <= 0) {
                settleParts(id);
            }
        }
        constexpr std::uint64_t kLookEveryMs = 5000; // how often the large pictures are looked at, ms
        if (nowMs - m_lookedAtMs < kLookEveryMs) {
            return;
        }
        m_lookedAtMs = nowMs;
        for (auto& [id, recipe] : m_large) {
            const auto index = static_cast<std::size_t>(id);
            if (!recipe.freed && nowMs - m_lastUse[index] >= kFreeAfterMs) {
                m_inParts.erase(id); // a picture in parts is loaded whole when it is needed again
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

    std::optional<core::Bytes> PictureLibrary::readDataStart(const std::string& name, Source source, bool& jpeg, std::size_t most, std::size_t& fullSize) {
        jpeg = false;
        if (const auto file = m_files.find(core::toLower(name)); source != Source::Jpeg && file != m_files.end()) {
            std::ifstream in(file->second, std::ios::binary | std::ios::ate);
            if (in) {
                fullSize = static_cast<std::size_t>(in.tellg());
                core::Bytes data(std::min(fullSize, most));
                in.seekg(0);
                in.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
                if (in) {
                    return data;
                }
            }
        }
        const std::lock_guard<std::mutex> lock(m_archiveLock);
        for (auto archive = m_archives.rbegin(); archive != m_archives.rend(); ++archive) {
            const bool isJpeg = archive->kind() == formats::MrgArchive::Kind::Jpeg;
            if ((source == Source::Bitmap && isJpeg) || (source == Source::Jpeg && !isJpeg)) {
                continue;
            }
            if (auto data = archive->readStart(name, most, fullSize)) {
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
        if (m_partsWanted) {
            // a large map background is not decoded whole on a worker: find makes it a picture in parts
            auto known = m_partsKnown.find(key);
            if (known == m_partsKnown.end()) {
                bool jpeg = false;
                const auto data = readData(name, source, jpeg);
                const bool inParts = data && jpeg && data->size() >= kPartsFromBytes && formats::JpegBands::parse(*data).has_value();
                known = m_partsKnown.emplace(key, inParts).first;
            }
            if (known->second) {
                return false;
            }
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
            const core::LowPriorityScope low;
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
        // with quiet workers the first picture still decoding gets a few ms of this frame
        int slice = core::workerSliceMs();
        for (auto it = m_prefetch.begin(); it != m_prefetch.end(); ++it) {
            const int waitMs = slice;
            slice = 0;
            if (it->second.decoded.wait_for(std::chrono::milliseconds(waitMs)) == std::future_status::ready) {
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
        std::optional<Pixels> pixels;
        if ((m_defer || m_partsWanted) && m_replacements.empty()) {
            bool jpeg = false;
            // a picture handed out with only its size has just the start of its file read
            constexpr std::size_t kStartBytes = 2048; // a bitmap's header is 54 bytes; a JPEG's size comes after its tables
            if (m_defer) {
                std::size_t fullSize = 0;
                const auto start = readDataStart(name, source, jpeg, kStartBytes, fullSize);
                const bool whole = start && (start->size() == fullSize || (jpeg && m_partsWanted && fullSize >= kPartsFromBytes));
                if (start && !whole) {
                    if (const auto size = headerSize(*start, jpeg)) {
                        const int id = static_cast<int>(m_pictures.size());
                        m_pictures.push_back(Picture::deferred(size->width, size->height, this, id));
                        m_lastUse.push_back(m_clockMs);
                        m_loaded.emplace(key, id);
                        m_deferred.emplace(id, Recipe{name, tint, source});
                        return PictureId{id};
                    }
                }
            }
            std::optional<core::Bytes> data = readData(name, source, jpeg);
            if (data && jpeg && m_partsWanted) {
                // a large map background: the rows a draw shows are decoded then, the rest a few ms a beat
                if (const auto id = storeInParts(key, Recipe{name, tint, source}, *data)) {
                    return id;
                }
            }
            if (data && m_defer) {
                // only the size is read now (the layouts place things by it); the pixels are decoded and uploaded on first use
                if (const auto size = headerSize(*data, jpeg)) {
                    const int id = static_cast<int>(m_pictures.size());
                    m_pictures.push_back(Picture::deferred(size->width, size->height, this, id));
                    m_lastUse.push_back(m_clockMs);
                    m_loaded.emplace(key, id);
                    m_deferred.emplace(id, Recipe{name, tint, source});
                    return PictureId{id};
                }
            }
            pixels = loadFrom(name, tint, source, data, jpeg);
        } else {
            pixels = load(name, tint, source);
        }
        if (!pixels) {
            core::logWarning("picture ", name, " does not exist");
            m_missing.insert(key);
            return std::nullopt;
        }
        const Recipe recipe{name, tint, source};
        return store(key, *pixels, &recipe);
    }

    void PictureLibrary::loadDeferred(int id) {
        const auto found = m_deferred.find(id);
        if (found == m_deferred.end()) {
            return;
        }
        const Recipe recipe = found->second;
        m_deferred.erase(found);
        const auto index = static_cast<std::size_t>(id);
        const int width = m_pictures[index].width();
        const int height = m_pictures[index].height();
        const auto pixels = load(recipe.name, recipe.tint, recipe.source);
        auto texture = pixels ? Texture::create(m_device, pixels->image, pixels->detail) : std::nullopt;
        if (!texture) {
            core::logWarning("cannot load the picture ", recipe.name);
            m_pictures[index] = Picture{}; // drawn as nothing
            return;
        }
        if (texture->width() != width || texture->height() != height) {
            core::logWarning("picture ", recipe.name, " is ", texture->width(), "x", texture->height(), ", its header said ", width, "x", height);
        }
        m_pictures[index] = makePicture(std::move(*texture), *pixels);
        if (pixels->image.pixels().size() >= kLargePixels) {
            m_large.emplace(id, recipe);
        }
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
        const std::optional<core::Bytes> data = readData(name, source, jpeg);
        return loadFrom(name, tint, source, data, jpeg);
    }

    std::optional<PictureId> PictureLibrary::storeInParts(const std::string& key, const Recipe& recipe, core::Bytes& data) {
        if (data.size() < kPartsFromBytes) {
            return std::nullopt;
        }
        auto bands = formats::JpegBands::parse(data);
        auto texture = bands ? Texture::createOpaque(m_device, bands->width(), bands->height()) : std::nullopt;
        if (!texture) {
            return std::nullopt; // decoded whole, as before
        }
        const int id = static_cast<int>(m_pictures.size());
        const std::size_t pixelCount = static_cast<std::size_t>(bands->width()) * static_cast<std::size_t>(bands->height());
        core::logInfo("load: picture ", recipe.name, " ", bands->width(), "x", bands->height(), " is decoded in parts: ", bands->count(), " bands of ", bands->bandRows(), " rows");
        m_pictures.push_back(Picture::inParts(std::move(*texture), this, id));
        m_lastUse.push_back(m_clockMs);
        m_loaded.emplace(key, id);
        if (pixelCount >= kLargePixels) {
            m_large.emplace(id, recipe);
        }
        InParts parts;
        parts.recipe = recipe;
        parts.done.assign(static_cast<std::size_t>(bands->count()), 0);
        parts.left = bands->count();
        parts.bands = std::move(*bands);
        parts.data = std::move(data);
        m_inParts.emplace(id, std::move(parts));
        return PictureId{id};
    }

    void PictureLibrary::needRows(int id, int top, int bottom) {
        const auto found = m_inParts.find(id);
        if (found == m_inParts.end()) {
            return;
        }
        InParts& parts = found->second;
        const int rows = parts.bands.bandRows();
        const int count = parts.bands.count();
        const int first = std::clamp(top / rows, 0, count - 1);
        const int last = std::clamp((bottom + rows - 1) / rows, first + 1, count);
        parts.near = (first + last) / 2;
        decodeBands(id, parts, first, last);
        if (parts.left <= 0) {
            settleParts(id);
        }
    }

    void PictureLibrary::decodeBands(int id, InParts& parts, int first, int last) {
        thread_local std::vector<core::Color> rowsBuffer;
        const std::size_t width = static_cast<std::size_t>(parts.bands.width());
        for (int band = first; band < last; ++band) {
            if (parts.done[static_cast<std::size_t>(band)] != 0) {
                continue;
            }
            int end = band + 1; // one past the run of bands not decoded yet
            while (end < last && parts.done[static_cast<std::size_t>(end)] == 0) {
                ++end;
            }
            const int top = parts.bands.top(band);
            const int rows = parts.bands.top(end) - top;
            const std::size_t count = width * static_cast<std::size_t>(rows);
            rowsBuffer.resize(count);
            if (!parts.bands.decode(parts.data, band, end, rowsBuffer.data())) {
                core::logWarning("cannot decode rows ", top, " to ", top + rows, " of picture ", parts.recipe.name);
                std::fill(rowsBuffer.begin(), rowsBuffer.begin() + static_cast<std::ptrdiff_t>(count), core::Color{0, 0, 0, 255}); // opaque black
            }
            formats::addColor(rowsBuffer.data(), count, parts.recipe.tint);
            formats::quantizeTo16Bit(rowsBuffer.data(), count);
            Picture& picture = m_pictures[static_cast<std::size_t>(id)];
            if (band == 0) {
                picture.setTopLeft(rowsBuffer[0]);
            }
            picture.textureAsIs().setRows(top, rows, rowsBuffer.data());
            for (int i = band; i < end; ++i) {
                parts.done[static_cast<std::size_t>(i)] = 1;
            }
            parts.left -= end - band;
            band = end - 1;
        }
        constexpr std::size_t kKeptPixels = 1000000; // a larger buffer (everything left decoded at once) is given back
        if (rowsBuffer.capacity() > kKeptPixels) {
            std::vector<core::Color>().swap(rowsBuffer);
        }
    }

    void PictureLibrary::settleParts(int id) {
        const auto found = m_inParts.find(id);
        if (found == m_inParts.end()) {
            return;
        }
        core::logInfo("load: picture ", found->second.recipe.name, " has all its rows now");
        m_pictures[static_cast<std::size_t>(id)].partsDone();
        m_inParts.erase(found);
    }

    std::optional<PictureLibrary::Pixels> PictureLibrary::loadFrom(
        const std::string& name,
        core::Color tint,
        Source source,
        const std::optional<core::Bytes>& data,
        bool jpeg
    ) {
        (void)source; // the bytes are read already
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
