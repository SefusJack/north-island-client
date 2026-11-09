#include "client/services/settings/WindowMemory.h"

#include "client/services/settings/SettingsLines.h"

#include <algorithm>
#include <string>
#include <vector>

namespace client {

    std::filesystem::path WindowMemory::fileIn(const std::filesystem::path& userRoot) { return userRoot / "qol-window.ini"; }

    std::optional<WindowMemory> WindowMemory::load(const std::filesystem::path& file) {
        const std::vector<std::string> lines = readSettingsLines(file);
        const auto width = settingInt(lines, "Width");
        const auto height = settingInt(lines, "Height");
        if (!width || !height) {
            return std::nullopt;
        }
        WindowMemory memory;
        memory.width = std::max(*width, 800); // smallest window width in pixels
        memory.height = std::max(*height, 600); // smallest window height in pixels
        memory.maximized = settingInt(lines, "Maximized").value_or(0) == 1;
        return memory;
    }

    void WindowMemory::save(const std::filesystem::path& file) const {
        std::vector<std::string> lines{"[Window]"};
        setSettingValue(lines, "Width", std::to_string(width));
        setSettingValue(lines, "Height", std::to_string(height));
        setSettingValue(lines, "Maximized", maximized ? "1" : "0");
        writeSettingsLines(file, lines);
    }

}
