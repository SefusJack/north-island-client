#include "client/models/pets/ForageFile.h"

#include "core/FileSystem.h"

#include <string>
#include <vector>

namespace client {

    namespace {

        constexpr std::uint8_t kKey = 21; // xor key of the file's bytes

    }

    std::filesystem::path ForageFile::pathFor(const std::filesystem::path& userRoot, std::int32_t playerId) {
        return userRoot / (std::to_string(playerId) + "Forage");
    }

    std::optional<ForageFile> ForageFile::read(const std::filesystem::path& path) {
        const auto bytes = core::readFile(path);
        if (!bytes) {
            return std::nullopt;
        }
        std::vector<std::string> lines;
        std::size_t at = 0;
        while (at < bytes->size()) {
            const std::size_t length = (*bytes)[at++];
            std::string line;
            for (std::size_t i = 0; i < length && at < bytes->size(); ++i) {
                line += static_cast<char>((*bytes)[at++] ^ kKey);
            }
            lines.push_back(line);
        }
        if (lines.size() < 4) { // the four target lines
            return std::nullopt;
        }
        ForageFile file;
        for (std::size_t line = 0; line < file.items.size(); ++line) {
            int value = 0;
            for (const char digit : lines[line]) {
                if (digit < '0' || digit > '9') {
                    return std::nullopt;
                }
                value = value * 10 + (digit - '0'); // decimal base
            }
            file.items[line] = value;
        }
        return file;
    }

    bool ForageFile::write(const std::filesystem::path& path) const {
        core::Bytes bytes;
        for (const int item : items) {
            const std::string line = std::to_string(item);
            bytes.push_back(static_cast<std::uint8_t>(line.size()));
            for (const char character : line) {
                bytes.push_back(static_cast<std::uint8_t>(character) ^ kKey);
            }
        }
        return core::writeFile(path, bytes);
    }

}
