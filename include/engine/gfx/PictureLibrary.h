#ifndef NORTH_ISLAND_GFX_PICTURE_LIBRARY_H
#define NORTH_ISLAND_GFX_PICTURE_LIBRARY_H

#include "core/Color.h"
#include "formats/MrgArchive.h"
#include "gfx/Picture.h"
#include "gfx/PictureId.h"
#include "gfx/PixelArtPart.h"

#include <deque>
#include <cstdint>
#include <filesystem>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gfx {

    class RenderDevice;

    /// @brief The game's pictures by name ("Btn_Next", "LoadingPic", ...), from loose bitmaps and .BMg / .JMg archives.
    /// @details The first registration of a loose name wins; the archive registered last wins. A picture is decoded and
    /// uploaded the first time it is asked for; bitmaps get the game's colour rules (formats/GamePixels.h), then the
    /// device's pixel-art scaler when it has one (JPEGs are not scaled; a pack's picture is used as it is). Names are matched
    /// without regard to case.
    class PictureLibrary {
        public:
            /// @brief Creates an empty library.
            /// @param device The render device pictures are uploaded to.
            explicit PictureLibrary(
                const RenderDevice& device
            )
                : m_device(device) {}

            PictureLibrary(const PictureLibrary&) = delete;
            PictureLibrary& operator=(const PictureLibrary&) = delete;

            /// @brief Registers one loose bitmap file under its file name.
            /// @param bitmapFile The bitmap file.
            void addFile(const std::filesystem::path& bitmapFile);

            /// @brief Adds a pack picture (the qol client's mods) used instead of the game's picture of that name.
            /// @details The last one added for a name wins. It must have the original's size times detail (else the original is
            /// used and a warning logged); a name the game does not have is simply added. detail 2 is an HD picture (a pack's
            /// pictures\@2x\), drawn at the original's size with twice the pixels across.
            /// @param name The picture name, or a file name (loadFile).
            /// @param file The PNG, BMP or JPG file.
            /// @param detail How many of the file's pixels make one logical pixel.
            void addReplacement(const std::string& name, const std::filesystem::path& file, int detail = 1);

            /// @brief Registers all bitmap files of a directory.
            /// @param directory The directory.
            void addDirectory(const std::filesystem::path& directory);

            /// @brief Registers a .BMg or .JMg archive.
            /// @param archiveFile The archive file.
            /// @return True when the archive could be opened.
            bool addArchive(const std::filesystem::path& archiveFile);

            /// @brief Which archives a name may come from.
            /// @details Original: GetPicIndex / LoadMrgPic mode 0 for the .BMg archives, GetJpgIndex mode 1 for the .JMg ones.
            enum class Source { Any, Bitmap, Jpeg };

            /// @brief Finds a picture by name, loading it the first time.
            /// @param name The picture name.
            /// @param tint A colour added to the picture (the maps' tint); each tint is its own picture.
            /// @param source Which archives the name may come from.
            /// @return The picture id, or nothing when there is no such picture.
            std::optional<PictureId> find(const std::string& name, core::Color tint = {0, 0, 0, 0}, Source source = Source::Any);

            /// @brief Finds a picture as StretchDrawPic draws it at a size.
            /// @details Copied into a plain surface keyed on black, stretched with bilinear filtering, so a shape's edge darkens into its surroundings.
            /// @param name The picture name.
            /// @param width Target width.
            /// @param height Target height.
            /// @return The stretched picture's id, or nothing when there is no such picture.
            std::optional<PictureId> findStretched(const std::string& name, int width, int height);

            /// @brief Gives the pixels of a named picture with the colour rules applied, for pictures changed before they are shown (gauges); not kept.
            /// @param name The picture name.
            /// @param tint A colour added to the picture.
            /// @param source Which archives the name may come from.
            /// @return The pixels (an HD picture's shrunk to the logical size), or nothing.
            std::optional<formats::Image> decode(const std::string& name, core::Color tint = {0, 0, 0, 0}, Source source = Source::Any);

            /// @brief A picture's pixels, detail x detail of them to a logical pixel.
            struct Pixels {
                formats::Image image;
                int detail = 1;
                std::optional<formats::Image> logical;
            };

            /// @brief Gives the pixels at the picture's own detail (an HD pack picture's at 2), for a texture made from them (Texture::create's detail).
            /// @param name The picture name.
            /// @param tint A colour added to the picture.
            /// @param source Which archives the name may come from.
            /// @return The pixels, or nothing.
            std::optional<Pixels> decodeDetailed(const std::string& name, core::Color tint = {0, 0, 0, 0}, Source source = Source::Any) {
                return load(name, tint, source);
            }

            /// @brief A named picture's undecoded file bytes, read by the thread that owns the library so another thread can decode them.
            struct Raw {
                std::string name;
                core::Bytes data;
                bool jpeg = false;
            };

            /// @brief Finds a picture only when it is loaded already.
            /// @param name The picture name.
            /// @param tint A colour added to the picture.
            /// @param source Which archives the name may come from.
            /// @return The picture id, or nothing when it is not loaded (yet).
            std::optional<PictureId> findLoaded(const std::string& name, core::Color tint = {0, 0, 0, 0}, Source source = Source::Any) const;

            /// @brief Reads a picture's file bytes, to decode them off this thread with decodeRaw and upload them with storeDecoded.
            /// @details Nothing when the picture is loaded or known missing already, has no data, or a pack may replace it: find handles those.
            /// @param name The picture name.
            /// @param tint A colour added to the picture (part of its key).
            /// @param source Which archives the name may come from.
            /// @return The bytes, or nothing.
            std::optional<Raw> readRaw(const std::string& name, core::Color tint, Source source);

            /// @brief Decodes read bytes and applies the colour rules and the tint as find does.
            /// @details Touches no library state, so any thread may call it.
            /// @param raw The bytes from readRaw.
            /// @param tint A colour added to the picture.
            /// @return The pixels, or nothing when they cannot be decoded.
            static std::optional<formats::Image> decodeRaw(const Raw& raw, core::Color tint);

            /// @brief Uploads pixels decodeRaw made and files them under the key find uses, so find gives them from then on.
            /// @param raw The bytes the pixels came from.
            /// @param tint The tint they were decoded with.
            /// @param source The source they were read with.
            /// @param image The pixels, or nothing when decoding failed (the picture is then known missing).
            /// @return The picture id, or nothing.
            std::optional<PictureId> storeDecoded(const Raw& raw, core::Color tint, Source source, std::optional<formats::Image> image);

            /// @brief Reads and decodes a picture on a worker thread, so a later find only uploads it.
            /// @details find waits for a picture still decoding instead of decoding it again. Loaded, missing, unknown and pack pictures are left to find.
            /// @param name The picture name.
            /// @param tint A colour added to the picture.
            /// @param source Which picture set to read.
            /// @return True when a worker was started (cancelPrefetch forgets it if find never asks for it).
            bool prefetch(const std::string& name, core::Color tint = {0, 0, 0, 0}, Source source = Source::Any);

            /// @brief Uploads one prefetched picture whose decode has finished, at most one a frame for all the scenes
            /// (each party member's battle calls it too).
            /// @param nowMs The frame's time; a second call with the same time uploads nothing.
            void pumpPrefetch(std::uint64_t nowMs);

            /// @brief Forgets a prefetch that find has not used (waits for its worker to finish first).
            /// @param name Picture name, as given to prefetch.
            /// @param tint Tint, as given to prefetch.
            /// @param source Source, as given to prefetch.
            void cancelPrefetch(const std::string& name, core::Color tint = {0, 0, 0, 0}, Source source = Source::Any) { m_prefetch.erase(keyFor(name, tint, source)); }

            /// @brief Loads a picture file by path (a JPEG background, a bitmap outside the named sets).
            /// @details JPEGs are drawn opaque; bitmaps get the colour rules.
            /// @param file The picture file.
            /// @return The picture id, or nothing when it cannot be loaded.
            std::optional<PictureId> loadFile(const std::filesystem::path& file);

            /// @brief A loaded picture.
            /// @param id A valid picture id of this library.
            /// @return The picture.
            const Picture& picture(PictureId id) const {
                const auto index = static_cast<std::size_t>(id.value);
                if (index < m_lastUse.size()) {
                    m_lastUse[index] = m_clockMs;
                    if (m_freedCount != 0 && !m_pictures[index].texture().isValid()) {
                        const_cast<PictureLibrary*>(this)->reloadFreed(id.value); // a large picture collect freed: load it again
                    }
                }
                return m_pictures[index];
            }

            /// @brief Marks a picture used now without loading it, so collect keeps it while it is not drawn (the map under a battle).
            /// @param id The picture.
            void touch(PictureId id) const {
                if (const auto index = static_cast<std::size_t>(id.value); index < m_lastUse.size()) {
                    m_lastUse[index] = m_clockMs;
                }
            }

            /// @brief Frees the texture and pixel mask of large pictures (at least kLargePixels) not used for kFreeAfterMs; picture loads them again when asked.
            /// @details Call once a frame: it also keeps the clock picture uses are stamped with, and looks at the large pictures every few seconds.
            /// @param nowMs The current time, ms.
            void collect(std::uint64_t nowMs);

            static constexpr std::size_t kLargePixels = 2000000; // pictures this many pixels or more (8 MB of texture) may be freed
            static constexpr std::uint64_t kFreeAfterMs = 120000; // a large picture unused this long is freed, ms

            /// @brief The number of pictures loaded so far.
            /// @return The picture count.
            int pictureCount() const { return static_cast<int>(m_pictures.size()); }

        private:
            /// @brief A pack picture file and its detail.
            struct Replacement {
                std::filesystem::path file;
                int detail = 1;
            };

            /// @brief Decodes a named picture's pixels (colour rules, tint, pixel-art scaling).
            /// @param name The picture name.
            /// @param tint A colour added to the picture.
            /// @param source Which archives the name may come from.
            /// @return The pixels, or nothing.
            std::optional<Pixels> load(const std::string& name, core::Color tint, Source source);

            /// @brief Reads a named picture's file bytes from the loose files or the archives; prefetch workers call it too.
            /// @param name The picture name.
            /// @param source Which picture set to read.
            /// @param jpeg Set to whether the bytes are a JPEG.
            /// @return The bytes, or nothing.
            std::optional<core::Bytes> readData(const std::string& name, Source source, bool& jpeg);

            /// @brief Whether readData would find a picture, without reading it.
            bool hasData(const std::string& name, Source source) const;

            /// @brief The cache key of a named picture in a tint from a source.
            /// @param name The picture name.
            /// @param tint A colour added to the picture.
            /// @param source Which archives the name may come from.
            /// @return The key.
            static std::string keyFor(const std::string& name, core::Color tint, Source source);

            /// @brief Applies the colour rules: a JPEG gets the tint and the 16-bit reduction, a bitmap the picture rules with the tint.
            /// @param image The decoded pixels.
            /// @param jpeg Whether they came from a JPEG.
            /// @param tint A colour added to the picture.
            static void applyRules(formats::Image& image, bool jpeg, core::Color tint);

            /// @brief Applies the device's pixel-art scaler to a bitmap's pixels (colour rules already applied).
            /// @param image The pixels.
            /// @param part What kind of picture it is (the digit fonts go with the text).
            /// @return The scaled pixels.
            Pixels scaled(formats::Image image, PixelArtPart part = PixelArtPart::Pictures) const;

            /// @brief Uploads pixels as a picture under a key.
            /// @param key The cache key.
            /// @param pixels The pixels.
            /// @return The new picture id, or nothing when the upload fails.
            /// @brief How find made a picture, so a freed one can be loaded again.
            struct Recipe {
                std::string name;
                core::Color tint{0, 0, 0, 0};
                Source source = Source::Any;
                bool freed = false;
            };

            /// @param mask The pixel mask when a prefetch worker made it already; else it is made from the pixels here.
            std::optional<PictureId> store(
                const std::string& key,
                const Pixels& pixels,
                const Recipe* recipe = nullptr,
                std::optional<Picture::Mask> mask = std::nullopt
            );

            /// @brief The picture of a texture and the pixels it was made from (their logical size gives the pixel mask).
            static Picture makePicture(Texture texture, const Pixels& pixels);

            /// @brief Loads a picture collect freed into its old slot again.
            /// @param id The picture.
            void reloadFreed(int id);

            /// @brief Finds the pack replacement of a name, when it fits the original.
            /// @param name The picture name.
            /// @param original The original's pixels, to check the size against.
            /// @param tint A colour added to the picture, as the original's would be.
            /// @return The replacement pixels, or nothing.
            std::optional<Pixels> replacement(const std::string& name, const std::optional<formats::Image>& original, core::Color tint);

            const RenderDevice& m_device;
            std::unordered_map<std::string, std::filesystem::path> m_files;
            std::vector<formats::MrgArchive> m_archives;
            std::mutex m_archiveLock; // archive reads share each archive's file position; prefetch workers read too
            std::deque<Picture> m_pictures;
            std::unordered_map<std::string, int> m_loaded;
            std::unordered_set<std::string> m_missing;
            struct Decoded {
                Raw raw; // name and kind; the bytes are dropped once decoded
                std::optional<Pixels> pixels; // colour rules and pixel-art scaling done
                Picture::Mask mask; // of the pixels at the logical size
            };
            struct Prefetch {
                core::Color tint{0, 0, 0, 0};
                Source source = Source::Any;
                std::future<Decoded> decoded;
            };
            std::optional<PictureId> settlePrefetch(std::unordered_map<std::string, Prefetch>::iterator prefetch);
            std::unordered_map<std::string, Prefetch> m_prefetch; // by find's key
            std::unordered_map<std::string, Replacement> m_replacements;
            mutable std::vector<std::uint64_t> m_lastUse; // by picture: m_clockMs when picture last returned it
            std::unordered_map<int, Recipe> m_large; // pictures of at least kLargePixels that collect may free, by id
            std::size_t m_freedCount = 0; // large pictures freed now
            std::uint64_t m_clockMs = 0; // the time collect was last given, ms
            std::uint64_t m_lookedAtMs = 0; // when collect last looked at the large pictures, ms
            std::optional<std::uint64_t> m_pumpedAtMs; // the frame time of pumpPrefetch's last upload, ms
    };

}

#endif
