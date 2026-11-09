#ifndef NORTH_ISLAND_UI_LOAD_CONTEXT_H
#define NORTH_ISLAND_UI_LOAD_CONTEXT_H

#include "core/Color.h"
#include "core/Point.h"
#include "core/Rect.h"
#include "gfx/PictureId.h"
#include "gfx/PictureLibrary.h"
#include "gfx/TextStyle.h"

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace ui {

    /// @brief What elements use while a layout is read: the picture library, game file resolution and attribute value parsers.
    class LoadContext {
        public:
            /// @brief Creates a context.
            /// @param pictures The picture library.
            /// @param resolveFile Resolves a game file name to a path.
            LoadContext(
                gfx::PictureLibrary& pictures,
                std::function<std::filesystem::path(const std::string&)> resolveFile
            )
                : m_pictures(pictures),
                  m_resolveFile(std::move(resolveFile)) {}

            /// @brief A picture of the library by name.
            /// @param name The picture name (empty gives no picture).
            /// @return The picture id (invalid when not found).
            gfx::PictureId picture(const std::string& name) const;

            /// @brief A picture file relative to the game directory.
            /// @param relativePath The file path.
            /// @return The picture id (invalid when empty or not loadable).
            gfx::PictureId pictureFile(const std::string& relativePath) const;

            /// @brief The pixel data of a picture.
            /// @param id The picture id.
            /// @return The picture.
            const gfx::Picture& pictureData(gfx::PictureId id) const { return m_pictures.picture(id); }

            /// @brief Resolves a game file name.
            /// @param relativePath The file name relative to the game directory.
            /// @return The resolved path.
            std::filesystem::path file(const std::string& relativePath) const { return m_resolveFile(relativePath); }

            std::function<int(const std::string& text, std::size_t position)> emoteAt;

            /// @brief Parses an integer attribute.
            /// @param value The text.
            /// @return The number; throws LayoutError when it is not one.
            static int parseInt(const std::string& value);

            /// @brief Parses a true / false attribute.
            /// @param value The text.
            /// @return The boolean; throws LayoutError otherwise.
            static bool parseBool(const std::string& value);

            /// @brief Parses "x,y".
            /// @param value The text.
            /// @return The point.
            static core::Point parsePoint(const std::string& value);

            /// @brief Parses "x,y,w,h".
            /// @param value The text.
            /// @return The rectangle.
            static core::Rect parseRect(const std::string& value);

            /// @brief Parses a "#RRGGBB" colour.
            /// @param value The text.
            /// @return The colour; throws LayoutError when it is not one.
            static core::Color parseColor(const std::string& value);

            /// @brief Parses a text effect: Plain, Outline or Shadow.
            /// @param value The text.
            /// @return The effect; throws LayoutError for other names.
            static gfx::TextEffect parseEffect(const std::string& value);

            /// @brief Parses a comma separated list of integers.
            /// @param value The text.
            /// @param count The number of integers expected.
            /// @return The integers; throws LayoutError when the count differs.
            static std::vector<int> parseInts(const std::string& value, std::size_t count);

        private:
            gfx::PictureLibrary& m_pictures;
            std::function<std::filesystem::path(const std::string&)> m_resolveFile;
    };

}

#endif
