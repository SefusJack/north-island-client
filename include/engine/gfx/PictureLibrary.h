#ifndef NORTH_ISLAND_GFX_PICTURE_LIBRARY_H
#define NORTH_ISLAND_GFX_PICTURE_LIBRARY_H

#include "core/Color.h"
#include "formats/MrgArchive.h"
#include "gfx/Picture.h"
#include "gfx/PictureId.h"
#include "gfx/PixelArtPart.h"

#include <deque>
#include <filesystem>
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

            /// @brief Loads a picture file by path (a JPEG background, a bitmap outside the named sets).
            /// @details JPEGs are drawn opaque; bitmaps get the colour rules.
            /// @param file The picture file.
            /// @return The picture id, or nothing when it cannot be loaded.
            std::optional<PictureId> loadFile(const std::filesystem::path& file);

            /// @brief A loaded picture.
            /// @param id A valid picture id of this library.
            /// @return The picture.
            const Picture& picture(PictureId id) const { return m_pictures[static_cast<std::size_t>(id.value)]; }

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

            /// @brief Applies the device's pixel-art scaler to a bitmap's pixels (colour rules already applied).
            /// @param image The pixels.
            /// @param part What kind of picture it is (the digit fonts go with the text).
            /// @return The scaled pixels.
            Pixels scaled(formats::Image image, PixelArtPart part = PixelArtPart::Pictures) const;

            /// @brief Uploads pixels as a picture under a key.
            /// @param key The cache key.
            /// @param pixels The pixels.
            /// @return The new picture id, or nothing when the upload fails.
            std::optional<PictureId> store(const std::string& key, const Pixels& pixels);

            /// @brief Finds the pack replacement of a name, when it fits the original.
            /// @param name The picture name.
            /// @param original The original's pixels, to check the size against.
            /// @param tint A colour added to the picture, as the original's would be.
            /// @return The replacement pixels, or nothing.
            std::optional<Pixels> replacement(const std::string& name, const std::optional<formats::Image>& original, core::Color tint);

            const RenderDevice& m_device;
            std::unordered_map<std::string, std::filesystem::path> m_files;
            std::vector<formats::MrgArchive> m_archives;
            std::deque<Picture> m_pictures;
            std::unordered_map<std::string, int> m_loaded;
            std::unordered_set<std::string> m_missing;
            std::unordered_map<std::string, Replacement> m_replacements;
    };

}

#endif
