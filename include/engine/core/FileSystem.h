#ifndef NORTH_ISLAND_CORE_FILE_SYSTEM_H
#define NORTH_ISLAND_CORE_FILE_SYSTEM_H

#include "core/Bytes.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace core {

    /// @brief Reads a whole file.
    /// @param path File to read.
    /// @return The contents, nothing when it cannot be opened.
    std::optional<Bytes> readFile(const std::filesystem::path& path);

    /// @brief Reads a text file split into lines.
    /// @param path File to read.
    /// @return The lines, nothing when it cannot be opened.
    std::optional<std::vector<std::string>> readLines(const std::filesystem::path& path);

    /// @brief Writes lines to a file, each ended with CR LF; an existing file is replaced.
    /// @param path File to write.
    /// @param lines Lines to write.
    /// @return True when the file was written.
    bool writeLines(const std::filesystem::path& path, const std::vector<std::string>& lines);

    /// @brief Writes bytes to a file; an existing file is replaced.
    /// @param path File to write.
    /// @param data Bytes to write.
    /// @return True when the file was written.
    bool writeFile(const std::filesystem::path& path, const Bytes& data);

    /// @brief Finds base / relative where relative may use '\' or '/' and any letter case (the game names files with Windows rules).
    /// @param base Directory to start in.
    /// @param relative Relative name as the game writes it.
    /// @return The path of the existing entry when there is one, otherwise base / relative as written.
    std::filesystem::path resolvePath(const std::filesystem::path& base, const std::string& relative);

}

#endif
