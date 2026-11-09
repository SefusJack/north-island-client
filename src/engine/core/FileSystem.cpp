#include "core/FileSystem.h"

#include "core/StringUtil.h"

#include <fstream>
#include <iterator>
#include <system_error>

namespace fs = std::filesystem;

namespace core {

    std::optional<Bytes> readFile(const fs::path& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return std::nullopt;
        }
        return Bytes(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    std::optional<std::vector<std::string>> readLines(const fs::path& path) {
        const auto data = readFile(path);
        if (!data) {
            return std::nullopt;
        }
        return splitLines(toString(*data));
    }

    bool writeLines(const fs::path& path, const std::vector<std::string>& lines) {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file) {
            return false;
        }
        for (const std::string& line : lines) {
            file << line << "\r\n";
        }
        return static_cast<bool>(file);
    }

    bool writeFile(const fs::path& path, const Bytes& data) {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file) {
            return false;
        }
        file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        return static_cast<bool>(file);
    }

    fs::path resolvePath(const fs::path& base, const std::string& relative) {
        fs::path result = base;
        std::string normalized = relative;
        for (char& c : normalized) {
            if (c == '\\') {
                c = '/';
            }
        }
        for (const std::string& part : split(normalized, '/')) {
            if (part.empty() || part == ".") {
                continue;
            }
            fs::path exact = result / part;
            std::error_code error;
            if (fs::exists(exact, error)) {
                result = exact;
                continue;
            }
            bool found = false;
            for (const auto& entry : fs::directory_iterator(result, error)) {
                if (equalsIgnoreCase(entry.path().filename().string(), part)) {
                    result = entry.path();
                    found = true;
                    break;
                }
            }
            if (!found) {
                result = exact;
            }
        }
        return result;
    }

}
