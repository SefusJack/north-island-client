#include "formats/StartupInfo.h"

#include "core/StringUtil.h"

namespace formats {

    StartupInfo StartupInfo::parse(const std::vector<std::string>& lines) {
        StartupInfo info;
        if (!lines.empty()) {
            info.windowed = lines[0] == "WindowScreen";
        }
        if (lines.size() > 1) {
            info.simplified = lines[1] == "Simplified";
        }
        if (lines.size() > 2) {
            if (lines[2] == "HalfMemory") {
                info.memory = MemoryMode::Half;
            } else if (lines[2] == "ShortMemory") {
                info.memory = MemoryMode::Short;
            }
        }
        return info;
    }

    StartupInfo StartupInfo::fromArguments(const std::string& screen, const std::string& language, const std::string& memory) {
        StartupInfo info;
        const auto screenMode = core::parseInt(screen);
        info.windowed = !screenMode || *screenMode == 2; // screen argument 2 = windowed
        const auto languageMode = core::parseInt(language);
        info.simplified = languageMode && *languageMode == 1;
        const auto memoryMode = core::parseInt(memory);
        if (!memoryMode || *memoryMode == 1) {
            info.memory = MemoryMode::Half;
        } else if (*memoryMode != 0) {
            info.memory = MemoryMode::Short;
        }
        return info;
    }

}
