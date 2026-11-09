#include "client/services/settings/SettingsLines.h"

#include <algorithm>
#include <exception>
#include <fstream>

namespace client {

    namespace {

        /// @brief Tells whether a line is "name=..." for a name.
        /// @param line The line.
        /// @param name The setting's name.
        /// @return True when the line starts with "name=" and has something after it.
        bool isLineOf(const std::string& line, const std::string& name) {
            return line.size() > name.size() && line.compare(0, name.size() + 1, name + "=") == 0;
        }

    }

    std::vector<std::string> readSettingsLines(const std::filesystem::path& file) {
        std::vector<std::string> lines;
        std::ifstream in(file);
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            lines.push_back(line);
        }
        return lines;
    }

    void writeSettingsLines(const std::filesystem::path& file, const std::vector<std::string>& lines) {
        std::ofstream out(file, std::ios::binary);
        for (const std::string& line : lines) {
            out << line << "\r\n";
        }
    }

    std::optional<std::string> settingValue(const std::vector<std::string>& lines, const std::string& name) {
        for (const std::string& line : lines) {
            if (isLineOf(line, name)) {
                return line.substr(name.size() + 1);
            }
        }
        return std::nullopt;
    }

    std::optional<int> settingInt(const std::vector<std::string>& lines, const std::string& name) {
        const auto value = settingValue(lines, name);
        if (!value || value->empty()) {
            return std::nullopt;
        }
        try {
            return std::stoi(*value);
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }

    void setSettingValue(std::vector<std::string>& lines, const std::string& name, const std::string& value) {
        for (std::string& line : lines) {
            if (isLineOf(line, name)) {
                line = name + "=" + value;
                return;
            }
        }
        lines.push_back(name + "=" + value);
    }

    bool hasSettingLine(const std::vector<std::string>& lines, const std::string& line) { return std::find(lines.begin(), lines.end(), line) != lines.end(); }

}
