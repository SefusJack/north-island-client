#ifndef NORTH_ISLAND_FORMATS_MRG_ARCHIVE_H
#define NORTH_ISLAND_FORMATS_MRG_ARCHIVE_H

#include "core/Bytes.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace formats {

    /// @brief A picture archive (.BMg bitmaps, .JMg JPEGs, .TMg targa files).
    /// @details Layout: u16 entry count, then one 32-byte entry per file (a Pascal short string of up to 23 characters with the
    /// file name, i32 offset, i32 size), then the file contents. Names are matched without regard to case.
    class MrgArchive {
        public:
            /// @brief What the archive holds, which decides the file extension.
            enum class Kind { Bitmap, Jpeg, Targa };

            /// @brief Opens an archive and reads its entry table.
            /// @param path Archive path; the extension (.bmg, .jmg, .tmg) decides the kind.
            /// @return The archive, or nothing when the extension is unknown or the file cannot be read.
            static std::optional<MrgArchive> open(const std::filesystem::path& path);

            /// @brief The archive's kind.
            /// @return The kind.
            Kind kind() const { return m_kind; }

            /// @brief Whether the archive has a file.
            /// @param name File name without extension.
            /// @return True when present.
            bool contains(const std::string& name) const;

            /// @brief Reads a file's contents.
            /// @param name File name without extension.
            /// @return The bytes, or nothing when missing or unreadable.
            std::optional<core::Bytes> read(const std::string& name);

            /// @brief Lists the archive's files.
            /// @return Entry names without extension.
            std::vector<std::string> names() const;

        private:
            /// @brief One file of the archive.
            struct Entry {
                std::string name;
                std::uint32_t offset = 0;
                std::uint32_t size = 0;
            };

            /// @brief Builds the lower-case map key of a file.
            /// @param name File name without extension.
            /// @return Name plus the kind's extension, lower-cased.
            std::string keyFor(const std::string& name) const;

            std::ifstream m_file;
            Kind m_kind = Kind::Bitmap;
            std::map<std::string, Entry> m_entries;
    };

}

#endif
