#ifndef NORTH_ISLAND_FORMATS_DATA_ARCHIVE_H
#define NORTH_ISLAND_FORMATS_DATA_ARCHIVE_H

#include "core/Bytes.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>

namespace formats {

    /// @brief A data archive (.MMG) read by file name, case-insensitively.
    /// @details Original: Ground.MMG holds the maps, Wem.MMG the ground objects. Layout: the file contents, then one 29-byte entry per file (a Pascal short string of up to 20 characters with the file name, i32 offset, i32 size), then u16 entry count as the last two bytes. Names (with extension, "10017.map") are matched without regard to case.
    class DataArchive {
        public:
            /// @brief Opens an archive and reads its entry table.
            /// @param path Archive file.
            /// @return The archive, nothing when unreadable or malformed.
            static std::optional<DataArchive> open(const std::filesystem::path& path);

            /// @brief Tells whether the archive holds a file.
            /// @param fileName File name with extension.
            /// @return True when present.
            bool contains(const std::string& fileName) const;

            /// @brief Reads a file out of the archive.
            /// @param fileName File name with extension.
            /// @return The contents, nothing when absent or unreadable.
            std::optional<core::Bytes> read(const std::string& fileName);

            /// @brief Gives the number of files.
            /// @return The entry count.
            std::size_t size() const { return m_entries.size(); }

        private:
            /// @brief Where one file lies in the archive.
            struct Entry {
                std::uint32_t offset = 0;
                std::uint32_t size = 0;
            };

            std::ifstream m_file;
            std::map<std::string, Entry> m_entries;
    };

}

#endif
